/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
  Portions:
  Copyright 2008-2010 Intel Corp..

  Authors:
  Iain Holmes <iain@linux.intel.com>
  Roger WANG <roger.wang@intel.com>
*/

#include <cmath>
#include <ggadget/canvas_interface.h>
#include <ggadget/event.h>
#include <ggadget/logger.h>
#include <ggadget/scrolling_element.h>
#include <ggadget/slot.h>
#include <ggadget/texture.h>
#include <ggadget/view.h>
#include <ggadget/main_loop_interface.h>
#include <ggadget/element_factory.h>
#include "clutter_edit_element.h"
#include "clutter_edit_impl.h"

#define Initialize clutter_edit_element_LTX_Initialize
#define Finalize clutter_edit_element_LTX_Finalize
#define RegisterElementExtension clutter_edit_element_LTX_RegisterElementExtension

extern "C" {
  bool Initialize() {
    LOGI("Initialize clutter_edit_element extension.");
    return true;
  }

  void Finalize() {
    LOGI("Finalize clutter_edit_element extension.");
  }

  bool RegisterElementExtension(ggadget::ElementFactory *factory) {
    LOGI("Register clutter_edit_element extension.");
    if (factory) {
      factory->RegisterElementClass(
          "edit", &ggadget::clutter::ClutterEditElement::CreateInstance);
    }
    return true;
  }
}

namespace ggadget {
namespace clutter {

static const int kDefaultEditElementWidth = 60;
static const int kDefaultEditElementHeight = 16;
static const Color kDefaultBackgroundColor(1, 1, 1);

ClutterEditElement::ClutterEditElement(View *view, const char *name)
    : EditElementBase(view, name),
      impl_(new ClutterEditImpl(this, ggadget::GetGlobalMainLoop(),
                            kDefaultEditElementWidth,
                            kDefaultEditElementHeight)) {
  impl_->SetBackground(new Texture(kDefaultBackgroundColor, 1.0));
  ConnectOnScrolledEvent(NewSlot(this, &ClutterEditElement::OnScrolled));
}

ClutterEditElement::~ClutterEditElement() {
  delete impl_;
}

void ClutterEditElement::Layout() {
  EditElementBase::Layout();
  int range, line_step, page_step, cur_pos;

  impl_->SetWidth(static_cast<int>(ceil(GetClientWidth())));
  impl_->SetHeight(static_cast<int>(ceil(GetClientHeight())));
  impl_->GetScrollBarInfo(&range, &line_step, &page_step, &cur_pos);

  SetScrollYPosition(cur_pos);
  SetYLineStep(line_step);
  SetYPageStep(page_step);
  if (UpdateScrollBar(0, range)) {
    // If the scrollbar display state was changed, then call Layout()
    // recursively to redo Layout.
    Layout();
  }
}

void ClutterEditElement::MarkRedraw() {
  EditElementBase::MarkRedraw();
  impl_->MarkRedraw();
}

bool ClutterEditElement::HasOpaqueBackground() const {
  const Texture *background = impl_->GetBackground();
  return background && background->IsFullyOpaque();
}

Variant ClutterEditElement::GetBackground() const {
  return Variant(Texture::GetSrc(impl_->GetBackground()));
}

void ClutterEditElement::SetBackground(const Variant &background) {
  impl_->SetBackground(GetView()->LoadTexture(background));
}

bool ClutterEditElement::IsBold() const {
  return impl_->IsBold();
}

void ClutterEditElement::SetBold(bool bold) {
  impl_->SetBold(bold);
}

std::string ClutterEditElement::GetColor() const {
  return impl_->GetTextColor().ToString();
}

void ClutterEditElement::SetColor(const char *color) {
  impl_->SetTextColor(Color(color));
}

std::string ClutterEditElement::GetFont() const {
  return impl_->GetFontFamily();
}

void ClutterEditElement::SetFont(const char *font) {
  impl_->SetFontFamily(font);
}

bool ClutterEditElement::IsItalic() const {
  return impl_->IsItalic();
}

void ClutterEditElement::SetItalic(bool italic) {
  impl_->SetItalic(italic);
}

bool ClutterEditElement::IsMultiline() const {
  return impl_->IsMultiline();
}

void ClutterEditElement::SetMultiline(bool multiline) {
  impl_->SetMultiline(multiline);
}

std::string ClutterEditElement::GetPasswordChar() const {
  return impl_->GetPasswordChar();
}

void ClutterEditElement::SetPasswordChar(const char *passwordChar) {
  impl_->SetPasswordChar(passwordChar);
}

bool ClutterEditElement::IsStrikeout() const {
  return impl_->IsStrikeout();
}

void ClutterEditElement::SetStrikeout(bool strikeout) {
  impl_->SetStrikeout(strikeout);
}

bool ClutterEditElement::IsUnderline() const {
  return impl_->IsUnderline();
}

void ClutterEditElement::SetUnderline(bool underline) {
  impl_->SetUnderline(underline);
}

std::string ClutterEditElement::GetValue() const {
  return impl_->GetText();
}

void ClutterEditElement::SetValue(const char *value) {
  impl_->SetText(value);
}

bool ClutterEditElement::IsWordWrap() const {
  return impl_->IsWordWrap();
}

void ClutterEditElement::SetWordWrap(bool wrap) {
  impl_->SetWordWrap(wrap);
}

bool ClutterEditElement::IsReadOnly() const {
  return impl_->IsReadOnly();
}

void ClutterEditElement::SetReadOnly(bool readonly) {
  impl_->SetReadOnly(readonly);
}

bool ClutterEditElement::IsDetectUrls() const {
  // TODO
  return false;
}

void ClutterEditElement::SetDetectUrls(bool /*detect_urls*/) {
  // TODO
}

void ClutterEditElement::GetIdealBoundingRect(int *width, int *height) {
  int w, h;
  impl_->GetSizeRequest(&w, &h);
  if (width) *width = w;
  if (height) *height = h;
}

void ClutterEditElement::Select(int start, int end) {
  impl_->Select(start, end);
  // If the edit box has no focus, then the selection will be reset when it
  // gains the focus by clicking on it. Then the selection will be useless.
  // FIXME: Is it the correct behaviour?
  GetView()->SetFocus(this);
}

void ClutterEditElement::SelectAll() {
  impl_->SelectAll();
  // If the edit box has no focus, then the selection will be reset when it
  // gains the focus by clicking on it. Then the selection will be useless.
  // FIXME: Is it the correct behaviour?
  GetView()->SetFocus(this);
}

CanvasInterface::Alignment ClutterEditElement::GetAlign() const {
  return impl_->GetAlign();
}

void ClutterEditElement::SetAlign(CanvasInterface::Alignment align) {
  impl_->SetAlign(align);
}

  //TODO: fix this
CanvasInterface::VAlignment ClutterEditElement::GetVAlign() const {
  return impl_->GetVAlign();
}

void ClutterEditElement::SetVAlign(CanvasInterface::VAlignment align) {
  impl_->SetVAlign(align);
}

void ClutterEditElement::DoDraw(CanvasInterface *canvas) {
  impl_->Draw(canvas);
  DrawScrollbar(canvas);
}

EventResult ClutterEditElement::HandleMouseEvent(const MouseEvent &event) {
  if (EditElementBase::HandleMouseEvent(event) == EVENT_RESULT_HANDLED)
    return EVENT_RESULT_HANDLED;
  return impl_->OnMouseEvent(event);
}

EventResult ClutterEditElement::HandleKeyEvent(const KeyboardEvent &event) {
  return impl_->OnKeyEvent(event);
}

EventResult ClutterEditElement::HandleOtherEvent(const Event &event) {
  if (event.GetType() == Event::EVENT_FOCUS_IN) {
    impl_->FocusIn();
    return EVENT_RESULT_HANDLED;
  } else if(event.GetType() == Event::EVENT_FOCUS_OUT) {
    impl_->FocusOut();
    return EVENT_RESULT_HANDLED;
  }
  return EVENT_RESULT_UNHANDLED;
}

void ClutterEditElement::GetDefaultSize(double *width, double *height) const {
  ASSERT(width && height);

  *width = kDefaultEditElementWidth;
  *height = kDefaultEditElementHeight;
}

void ClutterEditElement::OnFontSizeChange() {
  impl_->OnFontSizeChange();
}

void ClutterEditElement::OnScrolled() {
  DLOG("ClutterEditElement::OnScrolled(%d)", GetScrollYPosition());
  impl_->ScrollTo(GetScrollYPosition());
}

BasicElement *ClutterEditElement::CreateInstance(View *view, const char *name) {
  return new ClutterEditElement(view, name);
}

} // namespace clutter
} // namespace ggadget
