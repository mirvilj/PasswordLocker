#include "ItemCardPanel.h"
#include "ThemeManager.h"
#include <wx/menu.h>

CardDropTarget::CardDropTarget(ItemCardPanel* owner)
    : m_owner(owner) {
}

bool CardDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& text) {
    if (m_owner && !text.empty()) {
        m_owner->HandleDrop(text.ToStdString());
        return true;
    }
    return false;
}

ItemCardPanel::ItemCardPanel(wxWindow* parent,
                             const VaultItem& item,
                             int displayIndex,
                             int totalCount,
                             std::function<void(const std::string&)> onUnlockClicked,
                             std::function<void(const std::string&)> onDeleteClicked,
                             std::function<void(const std::string&, int)> onMoveRelative,
                             std::function<void(const std::string&, const std::string&)> onDropReorder,
                             std::function<void(const std::string&, const std::string&)> onMoveToTab,
                             const std::vector<std::string>& availableTabs,
                             double zoomFactor)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE),
      m_item(item),
      m_index(displayIndex),
      m_totalCount(totalCount),
      m_zoom(zoomFactor),
      m_onUnlockClicked(onUnlockClicked),
      m_onDeleteClicked(onDeleteClicked),
      m_onMoveRelative(onMoveRelative),
      m_onDropReorder(onDropReorder),
      m_onMoveToTab(onMoveToTab),
      m_availableTabs(availableTabs),
      m_isDragging(false) {

    SetBackgroundColour(ThemeManager::Colors.cardBg);
    SetDropTarget(new CardDropTarget(this));
    SetupUI();
    SetupEvents(this);
}

void ItemCardPanel::SetupEvents(wxWindow* win) {
    win->Bind(wxEVT_LEFT_DOWN, &ItemCardPanel::OnLeftDown, this);
    win->Bind(wxEVT_MOTION, &ItemCardPanel::OnMouseMove, this);
    win->Bind(wxEVT_LEFT_UP, &ItemCardPanel::OnLeftUp, this);
    win->Bind(wxEVT_CONTEXT_MENU, &ItemCardPanel::OnContextMenu, this);

    win->Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent& e) {
        if (ThemeManager::CurrentTheme == ThemeMode::Dark) {
            SetBackgroundColour(wxColour(39, 53, 76));
        } else if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
            SetBackgroundColour(wxColour(20, 20, 20));
        } else {
            SetBackgroundColour(wxColour(248, 250, 252));
        }
        Refresh();
        e.Skip();
    });
    win->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent& e) {
        SetBackgroundColour(ThemeManager::Colors.cardBg);
        Refresh();
        e.Skip();
    });

    for (wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst(); node; node = node->GetNext()) {
        wxWindow* child = node->GetData();
        if (!dynamic_cast<wxButton*>(child)) {
            SetupEvents(child);
        }
    }
}

void ItemCardPanel::SetupUI() {
    const auto& th = ThemeManager::Colors;
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    int titlePt = std::max(8, static_cast<int>(12 * m_zoom));
    int timePt = std::max(7, static_cast<int>(10 * m_zoom));
    int bodyPt = std::max(7, static_cast<int>(9 * m_zoom));

    // Left drag handle indicator
    int barWidth = std::max(18, static_cast<int>(22 * m_zoom));
    wxPanel* dragBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(barWidth, -1));
    dragBar->SetBackgroundColour(th.buttonBg);
    wxBoxSizer* dragBarSizer = new wxBoxSizer(wxVERTICAL);
    dragBarSizer->AddStretchSpacer();
    wxStaticText* dragHandleTxt = new wxStaticText(dragBar, wxID_ANY, "||");
    dragHandleTxt->SetForegroundColour(th.textMuted);
    wxFont dhf = dragHandleTxt->GetFont();
    dhf.SetWeight(wxFONTWEIGHT_BOLD);
    dragHandleTxt->SetFont(dhf);
    dragBarSizer->Add(dragHandleTxt, 0, wxALIGN_CENTER);
    dragBarSizer->AddStretchSpacer();
    dragBar->SetSizer(dragBarSizer);
    mainSizer->Add(dragBar, 0, wxEXPAND);

    // Center Content
    wxBoxSizer* contentSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Label and Type Badge row
    wxBoxSizer* titleRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* labelTxt = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(m_item.label.c_str()));
    wxFont lf = labelTxt->GetFont();
    lf.SetPointSize(titlePt);
    lf.SetWeight(wxFONTWEIGHT_BOLD);
    labelTxt->SetFont(lf);
    labelTxt->SetForegroundColour(th.textPrimary);

    wxString typeStr = wxString::FromUTF8(AccountTypeToString(m_item.type).c_str());
    wxStaticText* typeBadge = new wxStaticText(this, wxID_ANY, " " + typeStr + " ");
    wxColour badgeBg = th.tagBg;
    wxColour badgeFg = th.tagText;

    if (ThemeManager::CurrentTheme == ThemeMode::HighContrast) {
        badgeBg = *wxBLACK;
        badgeFg = *wxWHITE;
    } else if (ThemeManager::CurrentTheme == ThemeMode::Dark) {
        if (m_item.type == AccountType::SecurityQuestions) {
            badgeBg = wxColour(120, 53, 15);
            badgeFg = wxColour(254, 243, 199);
        } else if (m_item.type == AccountType::Miscellaneous) {
            badgeBg = wxColour(88, 28, 135);
            badgeFg = wxColour(243, 232, 255);
        }
    } else {
        if (m_item.type == AccountType::SecurityQuestions) {
            badgeBg = wxColour(254, 243, 199);
            badgeFg = wxColour(180, 83, 9);
        } else if (m_item.type == AccountType::Miscellaneous) {
            badgeBg = wxColour(243, 232, 255);
            badgeFg = wxColour(126, 34, 206);
        }
    }
    typeBadge->SetBackgroundColour(badgeBg);
    typeBadge->SetForegroundColour(badgeFg);

    titleRow->Add(labelTxt, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    titleRow->Add(typeBadge, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    if (!m_item.totpSecret.empty()) {
        wxStaticText* totpBadge = new wxStaticText(this, wxID_ANY, " 2FA Active ");
        totpBadge->SetBackgroundColour(th.tagBg);
        totpBadge->SetForegroundColour(th.success);
        titleRow->Add(totpBadge, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    }

    if (!m_item.attachedImageBase64.empty()) {
        wxStaticText* imgBadge = new wxStaticText(this, wxID_ANY, " Document Attached ");
        imgBadge->SetBackgroundColour(th.tagBg);
        imgBadge->SetForegroundColour(th.warning);
        titleRow->Add(imgBadge, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    }

    contentSizer->Add(titleRow, 0, wxEXPAND | wxTOP | wxBOTTOM, 4);

    // 2. Bold Submission Timestamp row
    wxBoxSizer* timeRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText* timePrompt = new wxStaticText(this, wxID_ANY, "Submitted: ");
    timePrompt->SetForegroundColour(th.textMuted);
    wxFont pf = timePrompt->GetFont();
    pf.SetPointSize(timePt);
    timePrompt->SetFont(pf);

    // Bold time text!
    wxStaticText* boldTimeTxt = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(m_item.submittedTimeFormatted.c_str()));
    wxFont timeFont = boldTimeTxt->GetFont();
    timeFont.SetPointSize(timePt);
    timeFont.SetWeight(wxFONTWEIGHT_BOLD);
    boldTimeTxt->SetFont(timeFont);
    boldTimeTxt->SetForegroundColour(th.warning);

    wxStaticText* tabTxt = new wxStaticText(this, wxID_ANY, "   Section: " + wxString::FromUTF8(m_item.tabName.c_str()));
    tabTxt->SetForegroundColour(th.textSecondary);
    wxFont tf = tabTxt->GetFont();
    tf.SetPointSize(timePt);
    tabTxt->SetFont(tf);

    timeRow->Add(timePrompt, 0, wxALIGN_CENTER_VERTICAL);
    timeRow->Add(boldTimeTxt, 0, wxALIGN_CENTER_VERTICAL);
    timeRow->Add(tabTxt, 0, wxALIGN_CENTER_VERTICAL);
    contentSizer->Add(timeRow, 0, wxEXPAND | wxBOTTOM, 4);

    // 3. Hidden Content placeholder
    wxStaticText* hiddenTxt = new wxStaticText(this, wxID_ANY, "Protected Content (Click to unlock with PIN / Passphrase)");
    hiddenTxt->SetForegroundColour(th.textMuted);
    wxFont hf = hiddenTxt->GetFont();
    hf.SetPointSize(bodyPt);
    hiddenTxt->SetFont(hf);
    contentSizer->Add(hiddenTxt, 0, wxEXPAND | wxBOTTOM, 6);

    mainSizer->Add(contentSizer, 1, wxEXPAND | wxALL, 8);

    // Right Action Buttons
    wxBoxSizer* actionsSizer = new wxBoxSizer(wxHORIZONTAL);

    int btnH = std::max(22, static_cast<int>(24 * m_zoom));
    int upW = std::max(36, static_cast<int>(40 * m_zoom));
    int downW = std::max(44, static_cast<int>(48 * m_zoom));

    wxBoxSizer* arrowsSizer = new wxBoxSizer(wxVERTICAL);
    wxButton* upBtn = new wxButton(this, wxID_ANY, "Up", wxDefaultPosition, wxSize(upW, btnH));
    wxButton* downBtn = new wxButton(this, wxID_ANY, "Down", wxDefaultPosition, wxSize(downW, btnH));
    upBtn->SetToolTip("Move Up (or drag card)");
    downBtn->SetToolTip("Move Down (or drag card)");

    upBtn->Enable(m_index > 0);
    downBtn->Enable(m_index < m_totalCount - 1);

    std::string itemId = m_item.id;
    upBtn->Bind(wxEVT_BUTTON, [this, itemId](wxCommandEvent&) {
        if (m_onMoveRelative) m_onMoveRelative(itemId, -1);
    });
    downBtn->Bind(wxEVT_BUTTON, [this, itemId](wxCommandEvent&) {
        if (m_onMoveRelative) m_onMoveRelative(itemId, 1);
    });

    arrowsSizer->Add(upBtn, 0, wxBOTTOM, 2);
    arrowsSizer->Add(downBtn, 0);
    actionsSizer->Add(arrowsSizer, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    // Unlock & View button
    int unlockW = std::max(95, static_cast<int>(110 * m_zoom));
    int unlockH = std::max(28, static_cast<int>(32 * m_zoom));
    wxButton* unlockBtn = new wxButton(this, wxID_ANY, "Unlock & View", wxDefaultPosition, wxSize(unlockW, unlockH));
    unlockBtn->SetForegroundColour(th.accent);
    unlockBtn->Bind(wxEVT_BUTTON, [this, itemId](wxCommandEvent&) {
        if (m_onUnlockClicked) m_onUnlockClicked(itemId);
    });
    actionsSizer->Add(unlockBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    // Remove / Delete button
    int rmW = std::max(65, static_cast<int>(75 * m_zoom));
    wxButton* removeBtn = new wxButton(this, wxID_ANY, "Remove", wxDefaultPosition, wxSize(rmW, unlockH));
    removeBtn->SetForegroundColour(th.danger);
    removeBtn->Bind(wxEVT_BUTTON, [this, itemId](wxCommandEvent&) {
        if (m_onDeleteClicked) m_onDeleteClicked(itemId);
    });
    actionsSizer->Add(removeBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    mainSizer->Add(actionsSizer, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);

    SetSizer(mainSizer);
}

void ItemCardPanel::HandleDrop(const std::string& draggedItemId) {
    if (m_onDropReorder && draggedItemId != m_item.id) {
        m_onDropReorder(draggedItemId, m_item.id);
    }
}

void ItemCardPanel::OnLeftDown(wxMouseEvent& evt) {
    m_dragStartPos = evt.GetPosition();
    m_isDragging = false;
    evt.Skip();
}

void ItemCardPanel::OnMouseMove(wxMouseEvent& evt) {
    if (evt.Dragging() && evt.LeftIsDown()) {
        wxPoint pos = evt.GetPosition();
        if (abs(pos.x - m_dragStartPos.x) > 6 || abs(pos.y - m_dragStartPos.y) > 6) {
            StartDrag();
        }
    }
    evt.Skip();
}

void ItemCardPanel::OnLeftUp(wxMouseEvent& evt) {
    if (!m_isDragging) {
        wxPoint pos = evt.GetPosition();
        if (abs(pos.x - m_dragStartPos.x) < 6 && abs(pos.y - m_dragStartPos.y) < 6) {
            if (m_onUnlockClicked) {
                m_onUnlockClicked(m_item.id);
            }
        }
    }
    m_isDragging = false;
    evt.Skip();
}

void ItemCardPanel::StartDrag() {
    m_isDragging = true;
    wxTextDataObject data(wxString::FromUTF8(m_item.id.c_str()));
    wxDropSource dropSource(this);
    dropSource.SetData(data);
    dropSource.DoDragDrop(wxDrag_CopyOnly);
}

void ItemCardPanel::OnContextMenu(wxContextMenuEvent&) {
    wxMenu menu;
    menu.Append(101, "Unlock & View Details (PIN / Passphrase Required)");
    menu.AppendSeparator();
    menu.Append(102, "Move Up");
    menu.Append(103, "Move Down");

    wxMenu* subMenu = new wxMenu();
    int subId = 200;
    for (const auto& tab : m_availableTabs) {
        subMenu->Append(subId++, wxString::FromUTF8(tab.c_str()));
    }
    menu.AppendSubMenu(subMenu, "Move to Tab Section");

    menu.AppendSeparator();
    menu.Append(104, "Remove from Vault");

    if (m_index == 0) menu.Enable(102, false);
    if (m_index >= m_totalCount - 1) menu.Enable(103, false);

    std::string itemId = m_item.id;
    int selection = GetPopupMenuSelectionFromUser(menu);
    if (selection == 101) {
        if (m_onUnlockClicked) m_onUnlockClicked(itemId);
    } else if (selection == 102) {
        if (m_onMoveRelative) m_onMoveRelative(itemId, -1);
    } else if (selection == 103) {
        if (m_onMoveRelative) m_onMoveRelative(itemId, 1);
    } else if (selection == 104) {
        if (m_onDeleteClicked) m_onDeleteClicked(itemId);
    } else if (selection >= 200 && selection < 200 + static_cast<int>(m_availableTabs.size())) {
        std::string targetTab = m_availableTabs[selection - 200];
        if (m_onMoveToTab) m_onMoveToTab(itemId, targetTab);
    }
}
