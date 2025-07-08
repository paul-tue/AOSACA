#pragma once

#include <peak/peak.hpp>
#include "AOSACADlg.h"

class CCamera_IDS
{
private:
    CAOSACADlg  *m_pParent;
    HANDLE      m_ehCamThreadClose;
    HANDLE      m_ehCamThreadShutdown;
    DWORD       thdid_handle;
    HANDLE      thd_handle;
    long        m_nFrameSizeInBytes;

    BYTE *m_pImgBuff;
    BYTE *m_pBkgndBuff;

    // UEye camera specific
    std::shared_ptr<peak::Library>              m_pLibrary;
    std::shared_ptr<peak::core::NodeMap>        m_pNodeMapRemoteDevice;
    std::shared_ptr<peak::core::NodeMap>        m_pNodeMapSystem;
    std::shared_ptr<peak::core::NodeMap>        m_pNodeMapInterface;
    std::shared_ptr<peak::core::NodeMap>        m_pNodeMapLocalDevice;
    std::shared_ptr<peak::core::NodeMap>        m_pNodemapDataStream;
    std::shared_ptr<peak::core::Device>         m_pDevice;
    std::shared_ptr<peak::core::DataStream>     m_pDataStream;
    std::shared_ptr<peak::core::RemoteDevice>   m_pRemoteDevice;
    std::shared_ptr<peak::core::Buffer>         m_pBuffer;

    // Functions
    bool Camera_Initialization(void);
    void RotateImageCCW(unsigned char* in);
    void RotateImageCW(unsigned char* in);
    void FlipImageUD(unsigned char* in);
    void FlipImageLR(unsigned char* in);
    void antiTranspose(unsigned char* in);
    void Transpose(unsigned char* in);
    static DWORD WINAPI CamThread(LPVOID pParam);

public:
    CCamera_IDS(CAOSACADlg* parent);
    ~CCamera_IDS(void);
    void CatchFrame(void);
    bool UpdateExposureTime(void);
    bool UpdateCameraGain(void);
    void SubtractBackground(void);
    BOOL SaveImage(CStringA filename, bool bkgnd = false);
};

