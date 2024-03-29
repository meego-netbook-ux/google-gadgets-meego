/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

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

// UI constants.
// var kCategoryButtonHeight = category_active_img.height;
var kPluginBoxWidth = 240;
var kPluginBoxHeight = 100;
var kDefaultPluginRows = 2;
var kDefaultPluginColumns = 2;
var kCategoryGap = 15;
var kFixedExtraWidth = 62;
var kFixedExtraHeight = 153;
var kBorderMarginH = 12;
var kBorderMarginV = 30;
var kWindowMarginH = 12;
var kCategoryItemWidth = 140;

var kMinWidth = kFixedExtraWidth + 2 * kPluginBoxWidth + kBorderMarginH;
var kMinHeight = kFixedExtraHeight + 2 * kPluginBoxHeight + kBorderMarginV;

// Default layout: 714x555.
var gPluginRows = kDefaultPluginRows;
var gPluginColumns = kDefaultPluginColumns;
var gPluginsPerPage = gPluginRows * gPluginColumns;
var gPluginBoxGapX = 0;
var gPluginBoxGapY = 8;

var gCurrentCategory = "";
var gCurrentLanguage = "";
var gCurrentPageStartIndex = 0;
var gCurrentPlugins = [];

// Plugin download status.
var kDownloadStatusNone = 0;
var kDownloadStatusAdding = 1;
var kDownloadStatusAdded = 2;
var kDownloadStatusError = 3;
var kDownloadStatusOffline = 4;

function init() {
  LoadMetadata();
  UpdateLanguageBox();
}

function init_layout() {
  var category_width = 0;
  for (var i in kTopCategories) {
    var category = kTopCategories[i];
    category_item_ruler.value = GetDisplayCategory(category);
    var width = category_item_ruler.idealBoundingRect.width;
    category_width = Math.max(category_width, width);
  }
  for (var i in kBottomCategories) {
    var category = kBottomCategories[i];
    category_item_ruler.value = GetDisplayCategory(category);
    var width = category_item_ruler.idealBoundingRect.width;
    category_width = Math.max(category_width, width);
  }
  kCategoryItemWidth = category_width;
  category_width = category_width + 20;
  if (category_width > categories_div.offsetWidth) {
    categories_div.width = category_width;
    plugins_div.x = categories_div.offsetX + category_width + 10;
    /* plugin_info_div.x = plugins_div.offsetX; */
    kFixedExtraWidth = category_width + kWindowMarginH + 10;
    kMinWidth = kFixedExtraWidth + 3 * kPluginBoxWidth + kBorderMarginH;
    debug.trace("category_width:" + category_width);
    debug.trace("kFixedExtraWidth:" + kFixedExtraWidth);
  }
  view.removeElement(category_item_ruler);
}

function view_onopen() {
  var width, height;
  // init_layout();

  width = kFixedExtraWidth + 2 * kPluginBoxWidth + kBorderMarginH;
  height = kFixedExtraHeight + 2 * kPluginBoxHeight + kBorderMarginV;
  view.resizeTo(width, height);

  // We do the init in timer because gadgetBrowserUtils is not ready when
  // gadget is created and will be registered by c++ code right after the
  // gadget is created.
  setTimeout(init, 0);
}

function view_onclose() {
  ClearThumbnailTasks();
  ClearPluginDownloadTasks();
}

// Disable context menus.
function view_oncontextmenu() {
  event.returnValue = false;
}

function view_onsizing() {
  if (event.width < kMinWidth) {
    event.width = kMinWidth;
  }
  if (event.height < kMinHeight) {
    event.height = kMinHeight;
  }
}

function view_onsize() {
  var view_width = view.width;
  var view_height = view.height;
  bg_top_middle.width = view_width -
    bg_top_left.offsetWidth - bg_top_right.offsetWidth;
  bg_bottom_middle.width = view_width -
    bg_bottom_left.offsetWidth - bg_bottom_right.offsetWidth;
  bg_middle_left.height = view_height -
    bg_top_left.offsetHeight - bg_bottom_left.offsetHeight;
  bg_middle_middle.width = view_width -
    bg_middle_left.offsetWidth - bg_middle_right.offsetWidth;
  bg_middle_middle.height = view_height -
    bg_top_middle.offsetHeight - bg_bottom_middle.offsetHeight;
  bg_middle_right.height = view_height -
    bg_top_right.offsetHeight - bg_bottom_right.offsetHeight;

  window_body.width = view_width - kBorderMarginH;
  window_body.height = view_height - kBorderMarginV;
}

function window_onsize() {
  var plugins_width = window_body.width - kBorderMarginH;
  var plugins_height = window_body.height - kFixedExtraHeight + 20;
  var columns = Math.floor(plugins_width / kPluginBoxWidth);
  var rows = Math.floor(plugins_height / kPluginBoxHeight);
  language_box.height = Math.min(440, window_body.height - 30);

  if (typeof(plugins_div) != "undefined") {
    plugins_div.width = plugins_width;
    plugins_div.height = plugins_height;
    /* plugin_info_div.width = plugins_width - 6; */
    /* categories_div.height = plugins_height + 28; */
    gPluginBoxGapX = Math.floor((plugins_width - kPluginBoxWidth * columns) /
                                (columns + 1));
    gPluginBoxGapY = Math.floor((plugins_height - kPluginBoxHeight * rows) /
                                (rows + 1));
    if (rows != gPluginRows || columns != gPluginColumns) {
      gPluginRows = rows;
      gPluginColumns = columns;
      gPluginsPerPage = rows * columns;
      if (plugins_div.children.count > 0)
        SelectPage(gCurrentPageStartIndex);
    } else {
      var index = 0;
      for (var i = 0; i < rows; i++) {
        for (var j = 0; j < columns; j++) {
          if (index >= plugins_div.children.count)
            break;
          var box = plugins_div.children.item(index);
          box.x = Math.round(j * (kPluginBoxWidth + gPluginBoxGapX) +
                             gPluginBoxGapX / 2);
          box.y = Math.round(i * (kPluginBoxHeight + gPluginBoxGapY) +
                             gPluginBoxGapY / 2);
          index++;
        }
      }
    }
  }
}

function language_label_onsize() {
  language_box_div.x = language_label.offsetX +
                       language_label.offsetWidth + 7;
}

/* function plugin_description_onsize() { */
/*   var max_height = 2 * plugin_title.offsetHeight; */
/*   if (plugin_description.offsetHeight > max_height) { */
/*     plugin_description.height = max_height; */
/*     plugin_other_data.y = plugin_description.offsetY + max_height; */
/*   } else { */
/*     plugin_other_data.y = plugin_description.offsetY + */
/*                           plugin_description.offsetHeight + 2; */
/*   } */
/* } */

function page_label_onsize() {
  if (page_label.innerText) {
    page_label.width = page_label.offsetWidth + 40;
    page_label.x = next_button.x - page_label.width;
    previous_button.x = page_label.x;
    page_label.onsize = null;
  }
}

function previous_button_onclick() {
  if (gCurrentPageStartIndex > gPluginsPerPage) {
    SelectPage(gCurrentPageStartIndex - gPluginsPerPage);
  } else if (gCurrentPageStartIndex > 0) {
    SelectPage(0);
  }
}

function next_button_onclick() {
  if (gCurrentPageStartIndex <  gCurrentPlugins.length - gPluginsPerPage)
    SelectPage(gCurrentPageStartIndex + gPluginsPerPage);
}

function language_box_onchange() {
  if (!gUpdatingLanguageBox)
    SelectLanguage(language_box.children.item(language_box.selectedIndex).name);
}

function GetDisplayLanguage(language) {
  return strings["LANGUAGE_" + language.replace("-", "_").toUpperCase()];
}

function GetDisplayCategory(category) {
  return strings["CATEGORY_" + category.toUpperCase()];
}

var gUpdatingLanguageBox = false;
function UpdateLanguageBox() {
  gUpdatingLanguageBox = true;

  language_box.removeAllElements();
  var languages = [];
  for (var language in gPlugins) {
    if (language != kAllLanguage) {
      var disp_lang = GetDisplayLanguage(language);
      if (disp_lang)
        languages.push({lang: language, disp: disp_lang});
    }
  }
  languages.sort(function(a, b) { return a.disp.localeCompare(b.disp); });
  languages.unshift({
    lang: kAllLanguage,
    disp: GetDisplayLanguage(kAllLanguage)
  });

  var saved_lang = options.getValue ("language");
  debug.trace ("saved language: " + saved_lang);
  var selected = 0;

  for (var i = 0; i < languages.length; i++) {
    var language = languages[i].lang;
    language_box.appendElement(
      "<item name='" + language +
      "'><label vAlign='middle' size='10'>" + languages[i].disp +
      "</label></item>");
    if (language == saved_lang)
      selected = i;
  }
  language_box.selectedIndex = selected;
  if (saved_lang == "")
    SelectLanguage(kAllLanguage);
  else
    SelectLanguage (saved_lang);
  gUpdatingLanguageBox = false;
}

function SelectLanguage(language) {
  gCurrentLanguage = language;
  if (!gUpdatingLanguageBox) {
    if (typeof(plugins_div) == "undefined") {
      // when we start search in the welcome screen, we create the
      // plugin_div in this way
      populate_plugins_div ();
    }
    SelectCategory (kCategoryAll);
    options.putValue ("language", language);
  }
  // UpdateCategories();
}

function AddCategoryButton(category, y) {
  categories_div.appendElement(
    "<label x='10' width='" + kCategoryItemWidth + "' height='" +
    kCategoryButtonHeight + "' y='" + y +
    "' align='left' vAlign='middle' enabled='true' color='#292929' name='" +
    category + "' size='10' trimming='character-ellipsis'" +
    " onmouseover='category_onmouseover()' onmouseout='category_onmouseout()'" +
    " onclick='SelectCategory(\"" + category + "\")'>" +
    GetDisplayCategory(category) + "</label>");
}

function category_onmouseover() {
  category_hover_img.x = event.srcElement.offsetX;
  category_hover_img.y = event.srcElement.offsetY;
  category_hover_img.visible = true;
}

function add_btn_onmouseover(img) {
  img.src = "images/category_hover.png";
}

function add_btn_onmouseout(img) {
  img.src = "images/bg_button.png";
}

function category_onmouseout() {
  category_hover_img.visible = false;
}

function SelectCategory(category) {
  gCurrentCategory = category;
  if (category) {
    /* category_active_img.y = categories_div.children.item(category).offsetY; */
    /* category_active_img.visible = true; */
    gCurrentPlugins = GetPluginsOfCategory(gCurrentLanguage, gCurrentCategory);
    if (gCurrentPlugins.length == 0)
      welcome_no_updates ();
    else
      SelectPage(0);
    ResetSearchBox();
  } else {
    /* category_active_img.visible = false; */
    gCurrentPlugins = [];
  }
}

function AddPluginBox(plugin, index, row, column) {
  var x = Math.round(column * (kPluginBoxWidth + gPluginBoxGapX) +
                     gPluginBoxGapX / 2);
  var y = Math.round(row * (kPluginBoxHeight + gPluginBoxGapY) +
                     gPluginBoxGapY / 2);
  var info_url = GetPluginInfoURL(plugin);
  var box = plugins_div.appendElement(
    "<div x='" + x + "' y='" + y +
    "' width='" + kPluginBoxWidth + "' height='" + kPluginBoxHeight +
    "' enabled='true' onmouseover='pluginbox_onmouseover(" + index + ")'" +
    " onmouseout='pluginbox_onmouseout(" + index + ")'>" +
    " <img width='100%' height='100%' stretchMiddle='true'/>" + //item 0
    (info_url ? //plugin title, item 1
      " <a x='95' y='6' size='10' width='140' align='left' color='#292929'" +
      "  overColor='#292929' underline='false' trimming='character-ellipsis'" +
      "  onmouseover='plugin_title_onmouseover(" + index + ")'" +
      "  onmouseout='plugin_title_onmouseout(" + index + ")' height='30' wordwrap='true'/>" :
     " <label x='95' y='6' size='10' width='140' align='left' height='30'" +
      "  color='#292929' trimming='character-ellipsis' wordwrap='true'/>") +
    '<div width="140" height="45" x="95" y="40">' + //plugin
                                                    //description, item2
      '<label width="100%" height="100%" y="0" color="#8e8e8e" \n\
        size="8" trimming="character-ellipsis" wordwrap="true"          \n\
        />' +
    '</div>\n' +
    " <img x='0' y='48' opacity='70' src=''/>" +
    " <div x='11' y='6' width='80' height='83' background='#ffffff'>" +
    "  <img width='80' height='60' src='images/gadget-colored.png'" +
    "   cropMaintainAspect='true'/>" +
    " </div>" +
    " <button x='6' y='37' width='90' height='40' visible='false' size='10'" +
    "  color='#292929' stretchMiddle='true' trimming='character-ellipsis'" +
    "  downImage='images/add_button_down.png' " +
    "  overImage='images/add_button_hover.png'" +
    "  onmousedown='add_button_onmousedown(" + index + ")'" +
    "  onmouseover='add_button_onmouseover(" + index + ")'" +
    "  onmouseout='add_button_onmouseout(" + index + ")'" +
    "  onclick='add_button_onclick(" + index + ")'/>" +
    "</div>");

  // Set it here to prevent problems caused by special chars in the title.
  var title = box.children.item(1);
  title.innerText = GetPluginTitle(plugin, gCurrentLanguage);
  if (info_url)
    title.href = info_url;
  var desc = box.children.item(2).children.item(0);
  desc.innerText = GetPluginDescription(plugin, gCurrentLanguage);

  var thumbnail_element1 = box.children.item(4).children.item(0);
  if (plugin.source == 1) { // built-in gadgets
    thumbnail_element1.src = plugin.attributes.thumbnail_url;
  } else if (plugin.source == 2) { // from plugins.xml
    AddThumbnailTask(plugin, index, thumbnail_element1);
  }

  plugin.button = box.children.item(5);
  UpdateAddButtonVisualStatus(plugin);
}

function SetDownloadStatus(plugin, status) {
  plugin.download_status = status;

  var button = plugin.button;
  if (!button)
    return;

  try {
    // Test if the button has been destroyed.
    button.enabled = button.enabled;
  } catch (e) {
    plugin.button = null;
    // The button has been destroyed, so don't update its visual status.
    return;
  }
  UpdateAddButtonVisualStatus(plugin);
}

var kAddingStatusLabels = [
  strings.ADD_BUTTON_LABEL,
  strings.STATUS_ADDING,
  strings.STATUS_ADDED,
  strings.STATUS_ERROR_ADDING,
  strings.STATUS_OFFLINE,
];
var kUpdatingStatusLabels = [
  strings.UPDATE_BUTTON_LABEL,
  strings.STATUS_UPDATING,
  strings.STATUS_UPDATED,
  strings.STATUS_ERROR_UPDATING,
  strings.STATUS_OFFLINE,
];
var kStatusButtonImages = [
  "images/add_button_default.png",
  "images/status_adding_default.png",
  "images/status_added_default.png",
  "images/status_failed_default.png",
  "images/status_adding_default.png",
];

function UpdateAddButtonVisualStatus(plugin) {
  var button = plugin.button;
  var status = plugin.download_status;
  var labels = gCurrentCategory == kCategoryUpdates ?
               kUpdatingStatusLabels : kAddingStatusLabels;
  var offline = ! framework.system.network.online;

  if (offline) {
    button.caption   = strings.STATUS_OFFLINE;
    button.image     = kStatusButtonImages[kDownloadStatusOffline];
    button.overImage = button.image;
    button.visible   = plugin.mouse_over;
    return;
  }
  else {
    button.caption = labels[status];
    button.image = kStatusButtonImages[status];
  }
  // Don't disable the button when the download status is kDownloadStatusAdding
  // to ensure the button can get mouseout.

  if (status == kDownloadStatusNone) {
    button.overImage = "images/add_button_hover.png";
    button.visible = plugin.mouse_over;
  } else {
    button.visible = true;
    button.overImage = kStatusButtonImages[status];
  }
  button.downImage = status == kDownloadStatusAdding ?
      kStatusButtonImages[status] : "images/add_button_down.png";
}

function add_button_onmousedown(index) {
  // Reset the button status if the user clicks on it.
  if (gCurrentCategory != kCategoryUpdates)
    SetDownloadStatus(gCurrentPlugins[index], kDownloadStatusNone);
}

function add_button_onclick(index) {
  var plugin = gCurrentPlugins[index];
  if (plugin.download_status != kDownloadStatusAdding &&
      framework.system.network.online) {
    var is_updating = (gCurrentCategory == kCategoryUpdates);
    plugin.button = event.srcElement;
    SetDownloadStatus(plugin, kDownloadStatusAdding);
    if (gadgetBrowserUtils.needDownloadGadget(plugin.id)) {
      DownloadPlugin(plugin, is_updating);
    } else if (is_updating) {
      // The gadget has already been updated.
      SetDownloadStatus(plugin, kDownloadStatusAdded);
    } else {
      if (AddPlugin(plugin) >= 0)
        SetDownloadStatus(plugin, kDownloadStatusAdded);
      else
        SetDownloadStatus(plugin, kDownloadStatusNone);
    }
  }
}

function pluginbox_onmouseover(index) {
  MouseOverPlugin(event.srcElement, index);
}

function pluginbox_onmouseout(index) {
  MouseOutPlugin(event.srcElement, index);
}

function plugin_title_onmouseover(index) {
  MouseOverPlugin(event.srcElement.parentElement, index);
}

function plugin_title_onmouseout(index) {
  MouseOutPlugin(event.srcElement.parentElement, index);
}

function add_button_onmouseover(index) {
  if (gCurrentCategory != kCategoryUpdates &&
      gCurrentPlugins[index].download_status != kDownloadStatusAdding)
    SetDownloadStatus(gCurrentPlugins[index], kDownloadStatusNone);
  MouseOverPlugin(event.srcElement.parentElement, index);
}

function add_button_onmouseout(index) {
  if (gCurrentCategory != kCategoryUpdates &&
      gCurrentPlugins[index].download_status != kDownloadStatusAdding)
    SetDownloadStatus(gCurrentPlugins[index], kDownloadStatusNone);
  MouseOutPlugin(event.srcElement.parentElement, index);
}

function MouseOverPlugin(box, index) {
  var title = box.children.item(1);
  if (title.href) title.underline = true;

  box.children.item(0).src = "images/thumbnails_hover.png";
  // Show the "Add" button.
  box.children.item(5).visible = true;

  var plugin = gCurrentPlugins[index];
  /* plugin_title.innerText = GetPluginTitle(plugin, gCurrentLanguage); */
  /* plugin_description.innerText = GetPluginDescription(plugin, gCurrentLanguage); */
  /* plugin_description.height = undefined; */
  /* plugin_other_data.innerText = GetPluginOtherData(plugin); */
  plugin.mouse_over = true;
}

function MouseOutPlugin(box, index) {
  box.children.item(1).underline = false;
  box.children.item(0).src = "";
  // Hide the "Add" button when it's in normal state.
  if (!gCurrentPlugins[index].download_status)
    box.children.item(5).visible = false;
  /* plugin_title.innerText = ""; */
  /* plugin_description.innerText = ""; */
  /* plugin_other_data.innerText = ""; */
  gCurrentPlugins[index].mouse_over = false;
}

function GetTotalPages() {
  if (!gCurrentPlugins || !gCurrentPlugins.length) {
    // Return 1 instead of 0 to avoid '1 of 0'.
    return 1;
  }
  return Math.ceil(gCurrentPageStartIndex / gPluginsPerPage) +
         Math.ceil((gCurrentPlugins.length - gCurrentPageStartIndex) /
                   gPluginsPerPage);
}

// Note for start: We need to keep the first plugin in the page unchanged
// when user resizes the window, so the first plugin will not always be at
// the whole page boundary.
function SelectPage(start) {
  plugins_div.removeAllElements();
  /* plugin_title.innerText = ""; */
  /* plugin_description.innerText = ""; */
  /* plugin_other_data.innerText = ""; */

  ClearThumbnailTasks();
  gCurrentPageStartIndex = start;
  var page = Math.ceil(start / gPluginsPerPage);
outer:
  for (var r = 0; r < gPluginRows; r++) {
    for (var c = 0; c < gPluginColumns; c++) {
      var i = start + c;
      if (i >= gCurrentPlugins.length)
        break outer;
      AddPluginBox(gCurrentPlugins[i], i, r, c);
    }
    start += gPluginColumns;
  }
  page_label.innerText = strings.PAGE_LABEL.replace("{{PAGE}}", page + 1)
                         .replace("{{TOTAL}}", GetTotalPages());
  previous_button.visible = next_button.visible = page_label.visible = true;
  view.setTimeout(RunThumbnailTasks, 500);
}

function UpdateCategories() {
  category_hover_img.visible = category_active_img.visible = false;
  var y = 0;
  for (var i = categories_div.children.count - 1; i >= 2; i--)
    categories_div.removeElement(categories_div.children.item(i));
  var language_categories = gPlugins[gCurrentLanguage];
  if (!language_categories)
    return;

  for (var i in kTopCategories) {
    var category = kTopCategories[i];
    if (category in language_categories) {
      AddCategoryButton(category, y);
      y += kCategoryButtonHeight;
    }
  }
  y += kCategoryGap;
  for (var i in kBottomCategories) {
    var category = kBottomCategories[i];
    if (category in language_categories) {
      AddCategoryButton(category, y);
      y += kCategoryButtonHeight;
    }
  }
  SelectCategory(kCategoryAll);
}

function ResetSearchBox() {
  search_box.value = strings.SEARCH_GADGETS;
  search_box.color = "#808080";
  search_box.killFocus();
}

var kSearchDelay = 500;
var gSearchTimer = 0;
var gInFocusHandler = false;

function search_box_onfocusout() {
  gInFocusHandler = true
  if (search_box.value == "")
    ResetSearchBox();
  gInFocusHandler = false;
}

function search_box_onfocusin() {
  gInFocusHandler = true;
  if (search_box.value == strings.SEARCH_GADGETS) {
    search_box.value = "";
    search_box.color = "#000000";
  }
  gInFocusHandler = false;
}

var gSearchPluginsTimer = 0;
function search_box_onchange() {
  if (gSearchTimer) cancelTimer(gSearchTimer);
  if (gInFocusHandler || search_box.value == strings.SEARCH_GADGETS)
    return;

  if (search_box.value == "") {
    SelectCategory(kCategoryAll);
    // Undo the actions in ResetSearchBox() called by SelectCategory().
    search_box.focus();
  } else {
    if (gSearchPluginsTimer)
      view.clearTimeout(gSearchPluginsTimer);

    if (typeof(plugins_div) == "undefined") {
      // when we start search in the welcome screen, we create the
      // plugin_div in this way
      populate_plugins_div ();
    }

    gSearchPluginsTimer = setTimeout(function () {
      if (search_box.value && search_box.value != strings.SEARCH_GADGETS) {
        SelectCategory(null);
        gCurrentPlugins = SearchPlugins(search_box.value, gCurrentLanguage);
        SelectPage(0);
        gSearchPluginsTimer = 0;
      }
    }, kSearchDelay);
  }
}

function ui_exit() {
  if (view.confirm (strings.CONFIRM_EXIT)) {
    plugin.RemoveMe (true);
    framework.exit();
  }
}

function welcome_no_updates () {
  main_div.removeAllElements ();
  window_body.removeElement (navigation_div);
  main_div.appendElement (' \
	<img name="category_hover_img" opacity="55" width="200" height="23" \
             visible="false" src="images/category_hover.png" stretchMiddle="true"/> \
');
  main_div.appendElement (' \
	<img name="category_active_img" height="23" width="200" visible="false" \
             src="images/category_active.png" stretchMiddle="true"/> \
');
  main_div.appendElement (' \
      <label name="welcome_label" width="100%" height="50" \
       wordWrap="true" color="#292929" size="14">                              \
	&WELCOME_NO_UPDATES;                                            \
      </label>');

  main_div.appendElement (' \
	<img name="add_btn_img" width="180" pinX="50%" x="98" y="81" \
	     src="images/bg_button.png" stretchMiddle="true"/>');

  main_div.appendElement (' \
	<label name="welcome_add_gg" align="center" width="160" color="#0598c9" \
	       pinX="50%" pinY="50%" size="12" x="98" y="95" \
	       enabled="true" onclick="welcome_add_gadget(false)" \
	       onmouseover="add_btn_onmouseover(add_btn_img)" \
	       onmouseout="add_btn_onmouseout(add_btn_img)"> \
	  &WELCOME_ADD_GADGET; \
	</label> \
');
}

function populate_plugins_div () {
  main_div.removeAllElements ();
  main_div.appendElement ('<div name="plugins_div" width="100%" height="100%"/>');

  window_body.appendElement (' \
    <div name="navigation_div" width="350" height="44" x="100%" pinX="100%" \n\
      y="100%" pinY="100%">                                             \n\
      <button name="previous_button" pinX="100%" x="240"                \n\
        image="images/previous_default.png"                             \n\
        downImage="images/previous_down.png"                            \n\
        overImage="images/previous_hover.png"                           \n\
         onclick="previous_button_onclick()"/>                          \n\
      <label name="page_label" enabled="true" height="39" x="260"       \n\
        align="center" color="#292929" size="10" vAlign="middle"        \n\
        onsize="page_label_onsize()"/>                                  \n\
      <button name="next_button" x="280" image="images/next_default.png" \n\
        downImage="images/next_down.png" overImage="images/next_hover.png" \n\
        onclick="next_button_onclick()"/>                               \n\
    </div>');
}

function welcome_add_gadget(is_update) {

  populate_plugins_div ();

  if (is_update) {
    SelectCategory (kCategoryUpdates);
    gadgetBrowserUtils.scheduleMetaDataUpdate (0);
  } else
    SelectCategory(kCategoryAll);
  window_onsize ();
}
