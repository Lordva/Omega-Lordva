#include "reader_view.h"
#include "apps/i18n.h"

namespace Reader {

ReaderView::ReaderView() :
  View(),
  m_bufferTextView(KDFont::LargeFont, 0.5, 0.5, KDColorBlack),
  m_color(3),
  m_kdcolor(Palette::GreyWhite)
{
  m_bufferTextView.setText(I18n::translate(I18n::Message::ReaderApp));
}

void ReaderView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(KDRect(0, 0, bounds().width(), bounds().height()), m_kdcolor);
}

void ReaderView::reload() {
  switch (m_color) {
    case 0:
      m_kdcolor = Palette::GreyDark;
      break;
    case 1:
      m_kdcolor = Palette::GreyMiddle;
      break;
    case 2:
      m_kdcolor = Palette::GreyBright;
      break;
    default:
      m_kdcolor = Palette::GreyWhite;
  }

  m_bufferTextView.setBackgroundColor(m_kdcolor);

  markRectAsDirty(bounds());
}

void ReaderView::changeColor() {
  m_color++;
  if (m_color > 3)
    m_color = 0;

  reload();
}

int ReaderView::numberOfSubviews() const {
  return 1;
}

View * ReaderView::subviewAtIndex(int index) {
  return &m_bufferTextView;
}

void ReaderView::layoutSubviews(bool force) {
  m_bufferTextView.setFrame(KDRect(0, 0, bounds().width(), bounds().height()), force);
}

}

/** 
 * This is the view, it allows you to show an interface on your calc like rect, string... 
 * It follows an MVC pattern -> Model-View-Controller. The view allows you to show some changes on event
 * This file is linked to the C view (sample_view.h here). All files are linked -> app files.
**/
