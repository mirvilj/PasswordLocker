#ifndef IMAGE_VIEWER_DIALOG_H
#define IMAGE_VIEWER_DIALOG_H

#include <wx/wx.h>
#include <wx/image.h>
#include <string>

class ImageViewerDialog : public wxDialog {
public:
    ImageViewerDialog(wxWindow* parent,
                       const wxImage& image,
                       const std::string& imageName,
                       bool allowRemove = true);

    bool WasImageRemoved() const { return m_imageRemoved; }

private:
    wxImage m_originalImage;
    std::string m_imageName;
    bool m_allowRemove;
    bool m_imageRemoved;

    double m_zoomFactor;
    bool m_isFullScreen;
    wxPoint m_panOffset;
    bool m_isPanning;
    wxPoint m_lastMousePos;

    wxPanel* m_canvasPanel;
    wxStaticText* m_zoomLabel;
    wxButton* m_minimizeBtn;

    void SetupUI();
    void FitImageToScreen();
    void RenderImage(wxDC& dc);

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnKeyDown(wxKeyEvent& evt);
    void OnMouseWheel(wxMouseEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseUp(wxMouseEvent& evt);

    void OnZoomIn(wxCommandEvent& evt);
    void OnZoomOut(wxCommandEvent& evt);
    void OnFitPage(wxCommandEvent& evt);
    void OnToggleFullScreen(wxCommandEvent& evt);
    void OnRemoveImage(wxCommandEvent& evt);
    void OnCloseViewer(wxCommandEvent& evt);

    wxDECLARE_EVENT_TABLE();
};

#endif // IMAGE_VIEWER_DIALOG_H
