#include "ImageViewerDialog.h"
#include <wx/dcbuffer.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(ImageViewerDialog, wxDialog)
    EVT_CHAR_HOOK(ImageViewerDialog::OnKeyDown)
wxEND_EVENT_TABLE()

ImageViewerDialog::ImageViewerDialog(wxWindow* parent,
                                     const wxImage& image,
                                     const std::string& imageName,
                                     bool allowRemove)
    : wxDialog(parent, wxID_ANY, "Password Locker - Image Viewer", wxDefaultPosition, wxSize(900, 650),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
      m_originalImage(image),
      m_imageName(imageName),
      m_allowRemove(allowRemove),
      m_imageRemoved(false),
      m_zoomFactor(1.0),
      m_isFullScreen(true),
      m_panOffset(0, 0),
      m_isPanning(false),
      m_lastMousePos(0, 0),
      m_canvasPanel(nullptr),
      m_zoomLabel(nullptr),
      m_minimizeBtn(nullptr) {

    SetBackgroundColour(wxColour(11, 15, 25)); // Deep dark canvas
    SetupUI();

    // Bring into full screen automatically
    ShowFullScreen(true, wxFULLSCREEN_ALL);
    m_isFullScreen = true;

    CallAfter([this]() {
        FitImageToScreen();
        if (m_canvasPanel) m_canvasPanel->Refresh();
    });
}

void ImageViewerDialog::SetupUI() {
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Top Controls Bar
    wxPanel* topBar = new wxPanel(this, wxID_ANY);
    topBar->SetBackgroundColour(wxColour(17, 24, 39)); // Slate 900
    wxBoxSizer* barSizer = new wxBoxSizer(wxHORIZONTAL);

    wxString titleStr = m_imageName.empty() ? "Document Viewer" : wxString::FromUTF8(m_imageName.c_str());
    wxStaticText* titleLbl = new wxStaticText(topBar, wxID_ANY, titleStr);
    titleLbl->SetForegroundColour(*wxWHITE);
    wxFont tf = titleLbl->GetFont();
    tf.SetPointSize(11);
    tf.SetWeight(wxFONTWEIGHT_BOLD);
    titleLbl->SetFont(tf);
    barSizer->Add(titleLbl, 0, wxALIGN_CENTER_VERTICAL | wxALL, 10);

    barSizer->AddStretchSpacer();

    // Zoom controls
    wxButton* zoomOutBtn = new wxButton(topBar, wxID_ANY, " - ", wxDefaultPosition, wxSize(36, 28));
    zoomOutBtn->SetToolTip("Zoom Out");
    zoomOutBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnZoomOut, this);
    barSizer->Add(zoomOutBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    m_zoomLabel = new wxStaticText(topBar, wxID_ANY, "100%", wxDefaultPosition, wxSize(55, -1), wxALIGN_CENTER_HORIZONTAL);
    m_zoomLabel->SetForegroundColour(wxColour(203, 213, 225));
    barSizer->Add(m_zoomLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);

    wxButton* zoomInBtn = new wxButton(topBar, wxID_ANY, " + ", wxDefaultPosition, wxSize(36, 28));
    zoomInBtn->SetToolTip("Zoom In");
    zoomInBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnZoomIn, this);
    barSizer->Add(zoomInBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    wxButton* fitBtn = new wxButton(topBar, wxID_ANY, "Fit to Screen", wxDefaultPosition, wxSize(100, 28));
    fitBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnFitPage, this);
    barSizer->Add(fitBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);

    // Minimize / Toggle Fullscreen
    m_minimizeBtn = new wxButton(topBar, wxID_ANY, "Minimize (Esc)", wxDefaultPosition, wxSize(110, 28));
    m_minimizeBtn->SetToolTip("Exit fullscreen mode (or press Esc)");
    m_minimizeBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnToggleFullScreen, this);
    barSizer->Add(m_minimizeBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    // Remove Image button
    if (m_allowRemove) {
        wxButton* rmBtn = new wxButton(topBar, wxID_ANY, "Remove Image", wxDefaultPosition, wxSize(110, 28));
        rmBtn->SetForegroundColour(wxColour(239, 68, 68)); // Red text
        rmBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnRemoveImage, this);
        barSizer->Add(rmBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    }

    // Close button
    wxButton* closeBtn = new wxButton(topBar, wxID_CANCEL, "Close", wxDefaultPosition, wxSize(75, 28));
    closeBtn->Bind(wxEVT_BUTTON, &ImageViewerDialog::OnCloseViewer, this);
    barSizer->Add(closeBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);

    topBar->SetSizer(barSizer);
    mainSizer->Add(topBar, 0, wxEXPAND);

    // Canvas Panel
    m_canvasPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
    m_canvasPanel->SetBackgroundColour(wxColour(11, 15, 25));
    m_canvasPanel->SetDoubleBuffered(true);

    m_canvasPanel->Bind(wxEVT_PAINT, &ImageViewerDialog::OnPaint, this);
    m_canvasPanel->Bind(wxEVT_SIZE, &ImageViewerDialog::OnSize, this);
    m_canvasPanel->Bind(wxEVT_MOUSEWHEEL, &ImageViewerDialog::OnMouseWheel, this);
    m_canvasPanel->Bind(wxEVT_LEFT_DOWN, &ImageViewerDialog::OnMouseDown, this);
    m_canvasPanel->Bind(wxEVT_MOTION, &ImageViewerDialog::OnMouseMove, this);
    m_canvasPanel->Bind(wxEVT_LEFT_UP, &ImageViewerDialog::OnMouseUp, this);

    mainSizer->Add(m_canvasPanel, 1, wxEXPAND);
    SetSizer(mainSizer);
}

void ImageViewerDialog::FitImageToScreen() {
    if (!m_originalImage.IsOk() || !m_canvasPanel) return;

    wxSize canvasSize = m_canvasPanel->GetClientSize();
    int imgW = m_originalImage.GetWidth();
    int imgH = m_originalImage.GetHeight();

    if (imgW <= 0 || imgH <= 0 || canvasSize.x <= 0 || canvasSize.y <= 0) return;

    double scaleX = static_cast<double>(canvasSize.x - 40) / static_cast<double>(imgW);
    double scaleY = static_cast<double>(canvasSize.y - 40) / static_cast<double>(imgH);

    m_zoomFactor = std::min(scaleX, scaleY);
    if (m_zoomFactor > 3.0) m_zoomFactor = 3.0; // Don't over-magnify small icons
    if (m_zoomFactor < 0.05) m_zoomFactor = 0.05;

    m_panOffset = wxPoint(0, 0);

    if (m_zoomLabel) {
        m_zoomLabel->SetLabel(wxString::Format("%d%%", static_cast<int>(m_zoomFactor * 100.0)));
    }
}

void ImageViewerDialog::OnPaint(wxPaintEvent& evt) {
    if (!m_canvasPanel) return;
    wxAutoBufferedPaintDC dc(m_canvasPanel);
    RenderImage(dc);
}

void ImageViewerDialog::RenderImage(wxDC& dc) {
    wxSize canvasSize = m_canvasPanel->GetClientSize();
    dc.SetBackground(wxBrush(wxColour(11, 15, 25)));
    dc.Clear();

    if (!m_originalImage.IsOk()) {
        dc.SetTextForeground(wxColour(148, 163, 184));
        dc.DrawText("No image data available.", 40, 40);
        return;
    }

    int targetW = static_cast<int>(m_originalImage.GetWidth() * m_zoomFactor);
    int targetH = static_cast<int>(m_originalImage.GetHeight() * m_zoomFactor);

    if (targetW <= 0 || targetH <= 0) return;

    // Rescale image smoothly
    wxImage scaledImg = m_originalImage.Scale(targetW, targetH, wxIMAGE_QUALITY_HIGH);
    wxBitmap bmp(scaledImg);

    // Center in canvas + pan offset
    int posX = (canvasSize.x - targetW) / 2 + m_panOffset.x;
    int posY = (canvasSize.y - targetH) / 2 + m_panOffset.y;

    dc.DrawBitmap(bmp, posX, posY, true);
}

void ImageViewerDialog::OnSize(wxSizeEvent& evt) {
    evt.Skip();
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnKeyDown(wxKeyEvent& evt) {
    int key = evt.GetKeyCode();
    if (key == WXK_ESCAPE) {
        if (m_isFullScreen) {
            ShowFullScreen(false);
            m_isFullScreen = false;
            if (m_minimizeBtn) m_minimizeBtn->SetLabel("Maximize");
            FitImageToScreen();
            if (m_canvasPanel) m_canvasPanel->Refresh();
        } else {
            EndModal(wxID_CANCEL);
        }
        return;
    }
    evt.Skip();
}

void ImageViewerDialog::OnMouseWheel(wxMouseEvent& evt) {
    int rot = evt.GetWheelRotation();
    if (rot > 0) {
        m_zoomFactor = std::min(5.0, m_zoomFactor * 1.15);
    } else if (rot < 0) {
        m_zoomFactor = std::max(0.1, m_zoomFactor / 1.15);
    }
    if (m_zoomLabel) {
        m_zoomLabel->SetLabel(wxString::Format("%d%%", static_cast<int>(m_zoomFactor * 100.0)));
    }
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnMouseDown(wxMouseEvent& evt) {
    if (evt.LeftDown()) {
        m_isPanning = true;
        m_lastMousePos = evt.GetPosition();
        m_canvasPanel->SetCursor(wxCURSOR_SIZING);
    }
}

void ImageViewerDialog::OnMouseMove(wxMouseEvent& evt) {
    if (m_isPanning && evt.LeftIsDown()) {
        wxPoint currentPos = evt.GetPosition();
        m_panOffset.x += (currentPos.x - m_lastMousePos.x);
        m_panOffset.y += (currentPos.y - m_lastMousePos.y);
        m_lastMousePos = currentPos;
        m_canvasPanel->Refresh();
    }
}

void ImageViewerDialog::OnMouseUp(wxMouseEvent& evt) {
    m_isPanning = false;
    m_canvasPanel->SetCursor(wxCURSOR_ARROW);
}

void ImageViewerDialog::OnZoomIn(wxCommandEvent& evt) {
    m_zoomFactor = std::min(5.0, m_zoomFactor * 1.25);
    if (m_zoomLabel) m_zoomLabel->SetLabel(wxString::Format("%d%%", static_cast<int>(m_zoomFactor * 100.0)));
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnZoomOut(wxCommandEvent& evt) {
    m_zoomFactor = std::max(0.1, m_zoomFactor / 1.25);
    if (m_zoomLabel) m_zoomLabel->SetLabel(wxString::Format("%d%%", static_cast<int>(m_zoomFactor * 100.0)));
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnFitPage(wxCommandEvent& evt) {
    FitImageToScreen();
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnToggleFullScreen(wxCommandEvent& evt) {
    m_isFullScreen = !m_isFullScreen;
    ShowFullScreen(m_isFullScreen, wxFULLSCREEN_ALL);
    if (m_minimizeBtn) {
        m_minimizeBtn->SetLabel(m_isFullScreen ? "Minimize (Esc)" : "Full Screen");
    }
    FitImageToScreen();
    if (m_canvasPanel) m_canvasPanel->Refresh();
}

void ImageViewerDialog::OnRemoveImage(wxCommandEvent& evt) {
    int res = wxMessageBox("Are you sure you want to remove this attached image from the account?",
                           "Confirm Remove Image", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this);
    if (res == wxYES) {
        m_imageRemoved = true;
        EndModal(wxID_OK);
    }
}

void ImageViewerDialog::OnCloseViewer(wxCommandEvent& evt) {
    EndModal(wxID_CANCEL);
}
