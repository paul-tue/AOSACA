#include "StdAfx.h"
#include "AOSACA.h"
#include "Camera_IDS.h"
#include <sstream>
#include <fstream>
#include "FreeImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace peak;

extern AOSACAParams* g_AOSACAParams;

CCamera_IDS::CCamera_IDS(CAOSACADlg* parent)
{
	m_pParent = parent;
	m_pImgBuff = NULL;
	m_pBkgndBuff = NULL;

	try {
		// Initialize library
		m_pLibrary->Initialize();
	}

	catch (const std::exception& e)
	{
		g_AOSACAParams->g_stAppErrBuff = "No connection to IDS Library!\n\nCheck network configuration (incl. firewall settings) and close any other active camera applications.";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		peak::Library::Close();
	}

	if (g_AOSACAParams->g_bCamReady = Camera_Initialization())
	{	
		// Set camera thread and its events
		g_AOSACAParams->g_ehCamLive = CreateEventA(NULL, TRUE, FALSE, "WFS_CAM_LIVE_EVENT");
		g_AOSACAParams->g_ehCamSnap = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_SNAP_EVENT");
		g_AOSACAParams->g_ehCamNewFrame = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_NEW_FRAME_EVENT");
		m_ehCamThreadClose = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_THREAD_CLOSE_EVENT");
		m_ehCamThreadShutdown = CreateEventA(NULL, FALSE, FALSE, "WFS_CAM_THREAD_SHUTDOWN_EVENT");

		thd_handle = CreateThread(NULL, 0, CamThread, this, 0, &thdid_handle);
		SetThreadPriority(thd_handle, THREAD_PRIORITY_HIGHEST);
	}
}

CCamera_IDS::~CCamera_IDS(void)
// TODO: fix destructor (peak::internal::InternalErrorException)
{
	if (g_AOSACAParams->g_bCamReady)
	{
		if (g_AOSACAParams->g_frame_mode == LIVESHOW)
			ResetEvent(g_AOSACAParams->g_ehCamLive);
		SetEvent(m_ehCamThreadClose);
		::WaitForSingleObject(m_ehCamThreadShutdown, g_AOSACAParams->EXPOSURE_MS * 2);

		if (m_pNodeMapRemoteDevice)
		{
			// Stop acquisition
			auto acqStopNode = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop");
			if (acqStopNode && acqStopNode->IsAvailable() && acqStopNode->IsWriteable())
				acqStopNode->Execute();

			auto tlLockedNode = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked");
			if (tlLockedNode)
				tlLockedNode->SetValue(0);
		}

		try
		{
			if (m_pDataStream)
			{
				try 
				{
					m_pDataStream->StopAcquisition(peak::core::AcquisitionStopMode::Default);
				}
				catch (const peak::core::Exception& e) 
				{
					OutputDebugStringA(("Warning: StopAcquisition failed: " + std::string(e.what()) + "\n").c_str());
				}

				try 
				{
					m_pDataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);
				}
				catch (const peak::core::Exception& e) 
				{
					OutputDebugStringA(("Warning: Flush failed: " + std::string(e.what()) + "\n").c_str());
				}

				auto buffers = m_pDataStream->AnnouncedBuffers();
				for (const auto& buffer : buffers)
				{
					m_pDataStream->RevokeBuffer(buffer);
				}				

				try 
				{
					m_pDataStream.reset();  // <-- still throwing exception
				}
				catch (const peak::core::Exception& e) 
				{
					g_AOSACAParams->g_stAppErrBuff.Empty();
					g_AOSACAParams->g_stAppErrBuff = "Datastream could not be closed properly!";
					g_AOSACAParams->ShowError(MB_ICONERROR);
				}
			}
			try 
			{
				m_pDevice.reset();
			}
			catch (const peak::core::Exception& e) 
			{
				g_AOSACAParams->g_stAppErrBuff.Empty();
				g_AOSACAParams->g_stAppErrBuff = "Device could not be closed properly!";
				g_AOSACAParams->ShowError(MB_ICONERROR);
			}

		}
		catch (const peak::core::Exception& e)
		{
			OutputDebugStringA(("Unhandled exception in camera destructor: " + std::string(e.what()) + "\n").c_str());
		}

		CloseHandle(g_AOSACAParams->g_ehCamNewFrame);
		CloseHandle(g_AOSACAParams->g_ehCamSnap);
		CloseHandle(m_ehCamThreadClose);
		CloseHandle(m_ehCamThreadShutdown);

	}

	if (g_AOSACAParams->g_pImgBuffPrc)
		g_AOSACAParams->g_pImgBuffPrc = NULL;
	if (m_pImgBuff != NULL)
		delete[] m_pImgBuff;
	if (m_pBkgndBuff != NULL)
		delete[] m_pBkgndBuff;

	g_AOSACAParams->g_bCamReady = false;

}

bool CCamera_IDS::Camera_Initialization()
{
	auto& m_pDeviceManager = peak::DeviceManager::Instance();
	try
	{
		m_pDeviceManager.Update();
	}
	catch (const peak::core::Exception)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "The device manager could not be initialized.\nMake sure you installed the IDS libraries.";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}

	if (m_pDeviceManager.Devices().empty())
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "No camera found!\n\nCheck for devices with IDS peak cockpit!";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	// get the actual camera device
	try
	{
		m_pDevice = m_pDeviceManager.Devices().at(0)->OpenDevice(peak::core::DeviceAccessType::Control);
		// get the RemoteDevice NodeMap
		m_pNodeMapRemoteDevice = m_pDevice->RemoteDevice()->NodeMaps().at(0);
		// get the System NodeMap
		m_pNodeMapSystem = m_pDevice->ParentInterface()->ParentSystem()->NodeMaps().at(0);
		// get the Interface NodeMap
		m_pNodeMapInterface = m_pDevice->ParentInterface()->NodeMaps().at(0);
		// get the LocalDevice NodeMap
		m_pNodeMapLocalDevice = m_pDevice->NodeMaps().at(0);
	}
	catch (const std::exception& e)
	{
		g_AOSACAParams->g_stAppErrBuff = "Camera can not be opened!\n Make sure no other program is using the camera.";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		peak::Library::Close();
		return false;
	}

	// prepare data streams
	auto dataStreams = m_pDevice->DataStreams();
	if (dataStreams.empty())
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "No camera data streams available!";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}

	m_pDataStream = m_pDevice->DataStreams().at(0)->OpenDataStream();
	m_pNodemapDataStream = m_pDataStream->NodeMaps().at(0);
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("PixelFormat")->SetCurrentEntry("Mono8");

	// BINNING
	auto binningSelector = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("BinningSelector");
	binningSelector->SetCurrentEntry("Region0");  // binning happening on camera FPGA

	// Set binning modes (only if the camera supports this)
	auto binningHorizontalMode = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("BinningHorizontalMode");
	if (binningHorizontalMode && binningHorizontalMode->IsWriteable())
		binningHorizontalMode->SetCurrentEntry("Average");

	auto binningVerticalMode = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("BinningVerticalMode");
	if (binningVerticalMode && binningVerticalMode->IsWriteable())
		binningVerticalMode->SetCurrentEntry("Average");

	// Set binning factors
	auto binningHorizontal = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("BinningHorizontal");
	if (binningHorizontal && binningHorizontal->IsWriteable())
		binningHorizontal->SetValue(2);

	auto binningVertical = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("BinningVertical");
	if (binningVertical && binningVertical->IsWriteable())
		binningVertical->SetValue(2);

	// set ROI for buffer size (consider that binning is active!)
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("OffsetX")->SetValue(0);
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("OffsetY")->SetValue(0);
	try
	{
		m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Width")->SetValue(g_AOSACAParams->IMAGE_WIDTH_PIX);
	}
	catch (peak::core::OutOfRangeException)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Width setting out of range for this sensor!";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}
	try
	{
		m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Height")->SetValue(g_AOSACAParams->IMAGE_HEIGHT_PIX);
	}
		catch (peak::core::OutOfRangeException)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Height setting out of range for this sensor!";
		g_AOSACAParams->ShowError(MB_ICONERROR);
		return false;
	}

	if (m_pDataStream)
	{
		// Flush queue and prepare all buffers for revoking
		m_pDataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);

		// Clear all old buffers
		for (const auto& old_buffer : m_pDataStream->AnnouncedBuffers())
		{
			m_pDataStream->RevokeBuffer(old_buffer);
		}

		int64_t payloadSize = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")->Value();

		// Get number of minimum required buffers
		int numBuffersMinRequired = m_pDataStream->NumBuffersAnnouncedMinRequired();

		// Alloc buffers
		for (size_t count = 0; count < numBuffersMinRequired; count++)
		{
			auto buffer = m_pDataStream->AllocAndAnnounceBuffer(static_cast<size_t>(payloadSize), nullptr);
			m_pDataStream->QueueBuffer(buffer);
		}
	}


	// SET TRIGGER SOURCE "SOFTWARE" AND START INFINITE ACQUISITION
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSelector")->SetCurrentEntry("ExposureStart");
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerMode")->SetCurrentEntry("On");
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("TriggerSource")->SetCurrentEntry("Software");
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("AcquisitionMode")->SetCurrentEntry("Continuous");
	m_pDataStream->StartAcquisition(peak::core::AcquisitionStartMode::Default, peak::core::DataStream::INFINITE_NUMBER);
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")->SetValue(1);
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")->Execute();

	// disable all auto functions
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::EnumerationNode>("ExposureAuto")->SetCurrentEntry("Off");

	UpdateExposureTime();
	UpdateCameraGain();

	// make sure config file contains correct image dimensions
	// better: get image width / height automatically from camera directly
	// m_nFrameSizeInBytes = g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX * sizeof(unsigned char);
	auto width = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Width")->Value();
	auto height = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Height")->Value();
	m_nFrameSizeInBytes = width * height * sizeof(unsigned char);
	
	m_pImgBuff = new BYTE[m_nFrameSizeInBytes];
	ZeroMemory(m_pImgBuff, m_nFrameSizeInBytes);
	m_pBkgndBuff = new BYTE[m_nFrameSizeInBytes];
	ZeroMemory(m_pBkgndBuff, m_nFrameSizeInBytes);

	// debug test
	int expectedSize = g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX;
	if (m_nFrameSizeInBytes != expectedSize) {
		OutputDebugStringA("!!! SIZE MISMATCH !!!\n");
		OutputDebugStringA(("Expected: " + std::to_string(expectedSize) + "\n").c_str());
		OutputDebugStringA(("Payload:  " + std::to_string(m_nFrameSizeInBytes) + "\n").c_str());
	}

	// commented as this is only applicable for Baumer cameras, will implement later for IDS camera
	//Load background image into buffer if available
	/*
	CStringA filename = "utils\\background.bimg";
	ifstream bkgndFile;
	bkgndFile.open(filename, ios::in | ios::binary);
	if (!bkgndFile.read((char*)m_pBkgndBuff, m_nFrameSizeInBytes))
		ZeroMemory(m_pBkgndBuff, m_nFrameSizeInBytes);
	bkgndFile.close();
	*/

	return true;
}

bool CCamera_IDS::UpdateExposureTime(void)
{
	double exp_us;
	exp_us = g_AOSACAParams->EXPOSURE_MS * 1000;

	double minExposureTime = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->Minimum() / 1000;
	double maxExposureTime = m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->Maximum() / 1000;

	OutputDebugStringA(("Minimum exposure time: " + std::to_string(minExposureTime) + " ms\n").c_str());
	OutputDebugStringA(("Maximum exposure time: " + std::to_string(maxExposureTime) + " ms\n").c_str());
	OutputDebugStringA(("\nTrying to set new exposure time " + std::to_string(exp_us/1000) + " ms\n").c_str());

	/*
	if (exp_us > maxExposureTime)
	{
		exp_us = maxExposureTime;
	}
	if (exp_us < minExposureTime)
	{
		exp_us = minExposureTime;
		OutputDebugStringA(("Trying to set an exposure time too low\n " + std::to_string(1)).c_str());
	}
	*/
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(exp_us);

	return true;
}

bool CCamera_IDS::UpdateCameraGain(void)
{
	double gain = g_AOSACAParams->CAMGAIN_DB;
	try
	{
		m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::FloatNode>("Gain")->SetValue(gain);
	}
	catch (peak::core::OutOfRangeException)
	{
		g_AOSACAParams->g_stAppErrBuff.Empty();
		g_AOSACAParams->g_stAppErrBuff = "Selected Gain value out of range for this camera model!";
		g_AOSACAParams->ShowError(MB_ICONERROR);
	}

	return true;
}

void CCamera_IDS::SubtractBackground(void)
// TODO: adapt for IDS
{
	long  i;
	BYTE* buf_f_ptr;
	BYTE* buf_b_ptr;

	// Read the current spots on the screen into buf_f
	buf_f_ptr = g_AOSACAParams->g_pImgBuffPrc;
	buf_b_ptr = m_pBkgndBuff;

	for (i = 0; i < m_nFrameSizeInBytes; i++)
	{
		if (*(buf_f_ptr) >= *(buf_b_ptr))
			*(buf_f_ptr) = *(buf_f_ptr)-*(buf_b_ptr);
		else
			*(buf_f_ptr) = 0;

		buf_f_ptr++;
		buf_b_ptr++;
	}
}

void CCamera_IDS::CatchFrame(void)
{
	// trigger frame acquisition
	m_pNodeMapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("TriggerSoftware")->Execute();
	auto finishedBuffer = m_pDataStream->WaitForFinishedBuffer(100);

	if (finishedBuffer && finishedBuffer->HasImage())
	{
		// Get raw image pointer from the buffer
		const void* pRawImageData = (BYTE*)finishedBuffer->BasePtr();

		// Copy image to expected processing buffer (BYTE* destination)
		size_t payloadSize = finishedBuffer->Size();
		// memcpy(g_AOSACAParams->g_pImgBuffPrc, rawData, payloadSize);
		if (!g_AOSACAParams->g_pImgBuffPrc) {
			OutputDebugStringA("g_pImgBuffPrc is NULL!\n");
		}
		int expected = g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX;
		if (payloadSize > expected) {
			OutputDebugStringA("Payload size exceeds allocated buffer!\n");
		}

		memcpy(g_AOSACAParams->g_pImgBuffPrc, pRawImageData, payloadSize);
	}

	// queue buffer after image acquisition
	m_pDataStream->QueueBuffer(finishedBuffer);
}

//*************************************************************************************************
// not changed from original Camera.cpp
DWORD WINAPI CCamera_IDS::CamThread(LPVOID pParam)
{
	CCamera_IDS* parent = (CCamera_IDS*)pParam;
	HANDLE hCamEvents[3];
	bool bRunCamThread = true;
	CString text;
	double telapse;

	hCamEvents[0] = parent->m_ehCamThreadClose;
	hCamEvents[1] = g_AOSACAParams->g_ehCamLive;
	hCamEvents[2] = g_AOSACAParams->g_ehCamSnap;

	LARGE_INTEGER time1;
	LARGE_INTEGER time2;

	std::wofstream m_Logfile;
	//	m_Logfile.open("Clogfile.txt", std::wofstream::out);

	do
	{
		switch (::WaitForMultipleObjects(3, hCamEvents, FALSE, INFINITE))
		{
		case WAIT_OBJECT_0:
			bRunCamThread = false;
			//delete rawImage;
			//m_Logfile.close();
			break;
		case WAIT_OBJECT_0 + 1:
			//	m_Logfile<<"ELiveT\n";
			g_AOSACAParams->g_frame_mode = LIVESHOW;
			//Get current CPU clock time
			QueryPerformanceCounter(&time1);
			//Retrieve an image
			/*	error = parent->m_FCcam.FireSoftwareTrigger();
				error = parent->m_FCcam.RetrieveBuffer( rawImage );
				pImagePixels = rawImage->GetData();
				memcpy(g_AOSACAParams->g_pImgBuffPrc, pImagePixels, parent->m_nFrameSizeInBytes);			*/

			parent->CatchFrame();
			//Send window update signal to WFS Image dialog	
			parent->m_pParent->PostMessage(WM_UPDATE_WINDOW, 0, UPDATE_WFSIMG_WINDOW);
			//Get current CPU clock time
			QueryPerformanceCounter(&time2);
			telapse = (double)(g_AOSACAParams->m_lnFreq / (time2.QuadPart - time1.QuadPart));//ticks passed
			text.Format(_T("%2.2f"), telapse);
			(parent->m_pParent)->SetDlgItemText(IDE_FREQUENCY, text);
			//	m_Logfile<<"DLiveT\n";
			break;
		case WAIT_OBJECT_0 + 2:
			//	m_Logfile<<LPCTSTR(g_AOSACAParams->GetTimeStamp())<<"ESnapT\n";
			g_AOSACAParams->g_frame_mode = SNAPSHOT;
			/*	error = parent->m_FCcam.FireSoftwareTrigger();
				error = parent->m_FCcam.RetrieveBuffer( rawImage );
				pImagePixels = rawImage->GetData();
				memcpy(g_AOSACAParams->g_pImgBuffPrc, pImagePixels, parent->m_nFrameSizeInBytes);*/
			parent->CatchFrame();
			// g_AOSACAParams->g_bSubstractBkGnd ? parent->SubtractBackground() : 0;
			SetEvent(g_AOSACAParams->g_ehCamNewFrame);
			//	m_Logfile<<LPCTSTR(g_AOSACAParams->GetTimeStamp())<<"DSnapT\n";
			break;
		default:
			break;
		}
	} while (bRunCamThread);
	SetEvent(parent->m_ehCamThreadShutdown);

	return 0;
}

BOOL CCamera_IDS::SaveImage(CStringA filename, bool bkgnd)
{
	BOOL result = FALSE;
	g_AOSACAParams->g_frame_mode = WAIT;
	if (bkgnd)
		memcpy(m_pBkgndBuff, g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX * sizeof(BYTE));

	/*FlyCapture2::Image saveImage(g_AOSACAParams->IMAGE_HEIGHT_PIX,
										g_AOSACAParams->IMAGE_WIDTH_PIX,
										g_AOSACAParams->IMAGE_WIDTH_PIX,
										g_AOSACAParams->g_pImgBuffPrc,
										g_AOSACAParams->IMAGE_WIDTH_PIX*g_AOSACAParams->IMAGE_HEIGHT_PIX,
										PIXEL_FORMAT_MONO8, FlyCapture2::NONE);*/
										// now, we can create a FIBITMAP
	FIBITMAP* dib;
	dib = FreeImage_ConvertFromRawBits(g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX, g_AOSACAParams->IMAGE_HEIGHT_PIX, g_AOSACAParams->IMAGE_WIDTH_PIX, 8, 0, 0, 0, true);

	result = FreeImage_Save(FIF_TIFF, dib, filename, TIFF_NONE);
	// unload the FIBITMAP 
	FreeImage_Unload(dib);

	if (bkgnd)
	{
		memcpy(m_pBkgndBuff, g_AOSACAParams->g_pImgBuffPrc, g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX * sizeof(BYTE));
		CStringA filename = "utils\\background.bimg";
		ofstream bkgndFile;
		bkgndFile.open(filename, ios::out | ios::binary);
		bkgndFile.write((const char*)m_pBkgndBuff, g_AOSACAParams->IMAGE_WIDTH_PIX * g_AOSACAParams->IMAGE_HEIGHT_PIX);
		bkgndFile.close();
		result = true;
	}
	return result;
}