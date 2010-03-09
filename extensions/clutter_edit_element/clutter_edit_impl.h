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

#ifndef  GGADGET_CLUTTER_EDIT_IMPL_H__
#define  GGADGET_CLUTTER_EDIT_IMPL_H__

#include <functional>
#include <string>
#include <stack>
#include <vector>
#include <clutter/clutter.h>
#include <cairo.h>

#include <ggadget/common.h>
#include <ggadget/math_utils.h>
#include <ggadget/clip_region.h>
#include <ggadget/canvas_interface.h>

namespace ggadget {

class Texture;
class MainLoopInterface;

namespace clutter {

class CairoCanvas;
class CairoGraphics;
class ClutterEditElement;

/** ClutterEditImpl is the clutter implementation of EditElement */
class ClutterEditImpl {

 public:
  ClutterEditImpl(ClutterEditElement *owner,
              MainLoopInterface *main_loop,
              int width, int height);
  ~ClutterEditImpl();

  void Draw(CanvasInterface *canvas);
  EventResult OnMouseEvent(const MouseEvent &event);
  EventResult OnKeyEvent(const KeyboardEvent &event);
  void FocusIn();
  void FocusOut();
  void SetWidth(int width);
  int GetWidth();
  void SetHeight(int height);
  int GetHeight();
  void GetSizeRequest(int *width, int *height);
  void SetBold(bool bold);
  bool IsBold();
  void SetItalic(bool italic);
  bool IsItalic();
  void SetStrikeout(bool strikeout);
  bool IsStrikeout();
  void SetUnderline(bool underline);
  bool IsUnderline();
  void SetMultiline(bool multiline);
  bool IsMultiline();
  void SetWordWrap(bool wrap);
  bool IsWordWrap();
  void SetReadOnly(bool readonly);
  bool IsReadOnly();
  void SetText(const char *text);
  std::string GetText();
  void SetBackground(Texture *background);
  const Texture *GetBackground();
  void SetTextColor(const Color &color);
  Color GetTextColor();
  void SetFontFamily(const char *font);
  std::string GetFontFamily();
  void OnFontSizeChange();
  void SetPasswordChar(const char *c);
  std::string GetPasswordChar();
  bool IsScrollBarRequired();
  void GetScrollBarInfo(int *range, int *line_step,
                        int *page_step, int *cur_pos);
  void ScrollTo(int position);
  void MarkRedraw();
  /** Select text between start and end. */
  void Select(int start, int end);
  /** Select all text */
  void SelectAll();

  CanvasInterface::Alignment GetAlign() const;
  void SetAlign(CanvasInterface::Alignment align);
  CanvasInterface::VAlignment GetVAlign() const;
  void SetVAlign(CanvasInterface::VAlignment align);

 private:
  /**
   * Enum used to specify different motion types.
   */
  enum MovementStep {
    VISUALLY,
    WORDS,
    DISPLAY_LINES,
    DISPLAY_LINE_ENDS,
    PAGES,
    BUFFER
  };

  void QueueDraw();
  /** Remove the cached layout. */
  void ResetLayout();
  /**
   * Create pango layout on-demand. If the layout is not changed, return the
   * cached one.
   */
  PangoLayout* EnsureLayout();
  /** Create a new layout containning current edit content */
  PangoLayout* CreateLayout();
  /** Create cairo canvas on-demand. */
  CairoCanvas* EnsureCanvas();
  /** Adjust the scroll information */
  void AdjustScroll();
  /**
   * Send out a request to refresh all informations of the edit control
   * and queue a draw request.
   * If @c relayout is true then the layout will be regenerated.
   * */
  void QueueRefresh(bool relayout, bool adjust_scroll);
  /** Reset the input method context */
  void ResetImContext();
  /** Reset preedit text */
  void ResetPreedit();
  /** Create a new im context according to current visibility setting */
  void InitImContext();
  /** Set the visibility of the edit control */
  void SetVisibility(bool visible);

  /** Check if the cursor should be blinking */
  bool IsCursorBlinking();
  /** Send out a request to blink the cursor if necessary */
  void QueueCursorBlink();
  /** Timer callback to blink the cursor */
  bool CursorBlinkCallback(int timer_id);
  void ShowCursor();
  void HideCursor();

  /** Draw the Cursor to the canvas */
  void DrawCursor(CairoCanvas *canvas);
  /** Draw the text to the canvas */
  void DrawText(CairoCanvas *canvas);

  void GetCursorRects(Rectangle *strong, Rectangle *weak);

  void UpdateCursorRegion();

  void UpdateSelectionRegion();

  /** Move cursor */
  void MoveCursor(MovementStep step, int count, bool extend_selection);
  /** Move cursor visually, meaning left or right */
  int MoveVisually(int current_pos, int count);
  /** Move cursor in words */
  int MoveWords(int current_pos, int count);
  /** Move cursor in display lines */
  int MoveDisplayLines(int current_pos, int count);
  /** Move cursor in pages */
  int MovePages(int current_pos, int count);
  /** Move cursor to the beginning or end of a display line */
  int MoveLineEnds(int current_pos, int count);

  /** Set the current cursor offset, in number of characters. */
  void SetCursor(int cursor);
  /** Get the most reasonable character offset according to the pixel
   * coordinate in the layout */
  int XYToOffset(int x, int y);
  /** Get the offset range that is currently selected,in number of characters.*/
  bool GetSelectionBounds(int *start, int *end);
  /** Set the offest range that should be selected, in number of characters. */
  void SetSelectionBounds(int selection_bound, int cursor);

  /** Insert text at current caret position */
  void EnterText(const char *str);
  /** Delete text in a specified range, in number of characters. */
  void DeleteText(int start, int end);

  /** Select the current word under cursor */
  void SelectWord();
  /** Select the current display line under cursor */
  void SelectLine();
  /** Delete the text that is currently selected */
  void DeleteSelection();

  /** Cut the current selected text to the clipboard */
  void CutClipboard();
  /** Copy the current selected text to the clipboard */
  void CopyClipboard();
  /** Paste the text in the clipboard to current offset */
  void PasteClipboard();
  /** Delete a character before the offset of the cursor */
  void BackSpace();
  /** Delete a character at the offset of the cursor */
  void Delete();
  /** Switch between the overwrite mode and the insert mode*/
  void ToggleOverwrite();

  /** Gets the color of selection background. */
  Color GetSelectionBackgroundColor();
  /** Gets the color of selection text. */
  Color GetSelectionTextColor();

#if 0
  /**
   * Gets the clutter actor used by the gadget host and the cursor location
   * relative to the widget coordinate.
   */
  ClutterActor *GetActorAndCursorLocation(ClutterRectangle *cur);
#endif
  /**
   * Gets the cursor location in pango layout. The unit is pixel.
   */
  void GetCursorLocationInLayout(int *strong_x, int *strong_y, int *strong_height,
                                 int *weak_x, int *weak_y, int *weak_height);

#if 0
  /**
   * Updates the cursor location of input method context.
   */
  void UpdateIMCursorLocation();

  /** Callback function for IM "commit" signal */
  static void CommitCallback(GtkIMContext *context,
                             const char *str, void *gg);
  /** Callback function for IM "retrieve-surrounding" signal */
  static gboolean RetrieveSurroundingCallback(GtkIMContext *context,
                                              void *gg);
  /** Callback function for IM "delete-surrounding" signal */
  static gboolean DeleteSurroundingCallback(GtkIMContext *context, int offset,
                                            int n_chars, void *gg);
  /** Callback function for IM "preedit-start" signal */
  static void PreeditStartCallback(GtkIMContext *context, void *gg);
  /** Callback function for IM "preedit-changed" signal */
  static void PreeditChangedCallback(GtkIMContext *context, void *gg);
  /** Callback function for IM "preedit-end" signal */
  static void PreeditEndCallback(GtkIMContext *context, void *gg);
  /**
   * Callback for gtk_clipboard_request_text function.
   * This function performs real paste.
   */
  static void PasteCallback(GtkClipboard *clipboard,
                            const gchar *str, void *gg);
#endif

 private:
  /** Owner of this clutter edit implementation object. */
  ClutterEditElement *owner_;
  /** Main loop object */
  MainLoopInterface *main_loop_;
  /** Graphics object, must be CairoGraphics */
  const GraphicsInterface *graphics_;

  /** The CairoCanvas which hold cairo_t inside */
  CairoCanvas *canvas_;

#if 0
  /** Clutter InputMethod Context */
  GtkIMContext *im_context_;
#endif

  /** The cached Pango Layout */
  PangoLayout *cached_layout_;

  /** The text content of the edit control */
  std::string text_;
  /** The preedit text of the edit control */
  std::string preedit_;
  /** Attribute list of the preedit text */
  PangoAttrList *preedit_attrs_;
  /**
   *  The character that should be displayed in invisible mode.
   *  If this is empty, then the edit control is visible
   */
  std::string password_char_;

  /** Canvas width */
  int width_;
  /** Canvas height */
  int height_;

  /** The current cursor position in number of characters */
  int cursor_;
  /** The preedit cursor position within the preedit string */
  int preedit_cursor_;
  /**
   * The current selection bound in number of characters,
   * range between cursor_ and selection_bound_ are selected.
   */
  int selection_bound_;
  /** Length of current text in number of chars */
  int text_length_;

  /** X offset of current scroll, in pixels */
  int scroll_offset_x_;
  /** Y offset of current scroll, in pixels */
  int scroll_offset_y_;
  /** Timer id of cursor blink callback */
  int cursor_blink_timer_;
  /**
   * Indicates the status of cursor blinking,
   * 0 means hide cursor
   * otherwise means show cursor.
   * The maximum value would be 2, and decrased by one in each cursor blink
   * callback, then there would be 2/3 visible time and 1/3 invisible time.
   */
  int cursor_blink_status_;

  /** Whether the text is visible, decided by password_char_ */
  bool visible_;
  /** Whether the edit control is focused */
  bool focused_;
  /** Whether the input method should be reset */
  bool need_im_reset_;
  /** Whether the keyboard in overwrite mode */
  bool overwrite_;
  /** Whether the button click should select words */
  bool select_words_;
  /** Whether the button click should select lines */
  bool select_lines_;
  /** Whether the left button is pressed */
  bool button_;
  /** Whether the text should be bold */
  bool bold_;
  /** Whether the text should be underlined */
  bool underline_;
  /** Whether the text should be struck-out */
  bool strikeout_;
  /** Whether the text should be italic */
  bool italic_;
  /** Whether the text could be shown in multilines */
  bool multiline_;
  /** Whether the text should be wrapped */
  bool wrap_;
  /** whether the cursor should be displayed */
  bool cursor_visible_;
  /** whether the edit control is readonly */
  bool readonly_;
  /**
   * Indicates if the content of the edit control has been modified
   * since last draw
   */
  bool content_modified_;

  /** Indicates if the selection region has been changed since last draw. */
  bool selection_changed_;

  /** Indicates if the cursor position has been moved since last draw. */
  bool cursor_moved_;

  /** Indicates if the canvas cache needs updating. */
  bool update_canvas_;

  /** The font family of the text */
  std::string font_family_;
  /** The font size of the text */
  double font_size_;
  /** The background texture of the edit control */
  Texture *background_;
  /** The text color of the edit control */
  Color text_color_;

  CanvasInterface::Alignment align_;
  CanvasInterface::VAlignment valign_;

  ClipRegion last_selection_region_;
  ClipRegion selection_region_;
  ClipRegion last_cursor_region_;
  ClipRegion cursor_region_;

  DISALLOW_EVIL_CONSTRUCTORS(ClutterEditImpl);
};  // class ClutterEditImpl

} // namespace clutter
} // namespace ggadget

#endif   // GGADGET_CLUTTER_EDIT_IMPL_H__
