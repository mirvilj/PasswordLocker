#ifndef ITEM_CARD_PANEL_H
#define ITEM_CARD_PANEL_H

#include <wx/wx.h>
#include <wx/dnd.h>
#include "VaultItem.h"

// Custom drop target for card reordering
class CardDropTarget : public wxTextDropTarget {
public:
    CardDropTarget(class ItemCardPanel* owner);
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text) override;

private:
    class ItemCardPanel* m_owner;
};

class ItemCardPanel : public wxPanel {
public:
    ItemCardPanel(wxWindow* parent,
                  const VaultItem& item,
                  int displayIndex,
                  int totalCount,
                  std::function<void(const std::string&)> onUnlockClicked,
                  std::function<void(const std::string&)> onDeleteClicked,
                  std::function<void(const std::string&, int)> onMoveRelative,
                  std::function<void(const std::string&, const std::string&)> onDropReorder,
                  std::function<void(const std::string&, const std::string&)> onMoveToTab,
                  const std::vector<std::string>& availableTabs,
                  double zoomFactor = 1.0);

    const std::string& GetItemId() const { return m_item.id; }
    void HandleDrop(const std::string& draggedItemId);

private:
    VaultItem m_item;
    int m_index;
    int m_totalCount;
    double m_zoom;
    std::function<void(const std::string&)> m_onUnlockClicked;
    std::function<void(const std::string&)> m_onDeleteClicked;
    std::function<void(const std::string&, int)> m_onMoveRelative;
    std::function<void(const std::string&, const std::string&)> m_onDropReorder;
    std::function<void(const std::string&, const std::string&)> m_onMoveToTab;
    std::vector<std::string> m_availableTabs;

    bool m_isDragging;
    wxPoint m_dragStartPos;

    void SetupUI();
    void SetupEvents(wxWindow* win);
    void StartDrag();

    void OnLeftDown(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnContextMenu(wxContextMenuEvent& evt);
};

#endif // ITEM_CARD_PANEL_H
