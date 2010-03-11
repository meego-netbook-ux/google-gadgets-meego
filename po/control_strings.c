 #include "config.h" 

#include <locale.h> 
#include <stdio.h> 
                           #include <string.h> 
#include <libxml/encoding.h> 
#include <libxml/xmlwriter.h> 
#include <glib/gi18n.h> 

int 
main (int argc, char* argv[]) 
{ 
  setlocale (LC_ALL, argv[2]); 
  bindtextdomain (GETTEXT_PACKAGE, "locale");
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE); 

  xmlTextWriterPtr writer; 
                     xmlChar *tmp; 

  writer = xmlNewTextWriterFilename (argv[1], 0); 
  xmlTextWriterStartDocument (writer, NULL, "utf-8", NULL); 
  xmlTextWriterStartElement (writer, "strings"); 
xmlTextWriterStartElement (writer, "GADGET_NAME");
xmlTextWriterWriteString (writer, _("Gadgets Control"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "GADGET_DESCRIPTION");
xmlTextWriterWriteString (writer, _("Gadgets Control"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "GADGET_ABOUT_TEXT");
xmlTextWriterWriteString (writer, _("Gadgets Control"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "SHOW_ONLY_LABEL");
xmlTextWriterWriteString (writer, _("Show only:"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "PAGE_LABEL");
xmlTextWriterWriteString (writer, _("{{PAGE}} of {{TOTAL}}"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "ADD_BUTTON_LABEL");
xmlTextWriterWriteString (writer, _("Add"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_ADDING");
xmlTextWriterWriteString (writer, _("Adding..."));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_ADDED");
xmlTextWriterWriteString (writer, _("Added"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_ERROR_ADDING");
xmlTextWriterWriteString (writer, _("Error"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_OFFLINE");
xmlTextWriterWriteString (writer, _("Offline"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "UPDATE_BUTTON_LABEL");
xmlTextWriterWriteString (writer, _("Update"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_UPDATING");
xmlTextWriterWriteString (writer, _("Updating..."));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_UPDATED");
xmlTextWriterWriteString (writer, _("Updated"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "STATUS_ERROR_UPDATING");
xmlTextWriterWriteString (writer, _("Error"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "SEARCH_GADGETS");
xmlTextWriterWriteString (writer, _("Search gadgets"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CONFIRM_EXIT");
xmlTextWriterWriteString (writer, _("Do you want to close the Gadgets Zone? This will remove all your installed gadgets."));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "WELCOME_TO_GADGETS");
xmlTextWriterWriteString (writer, _("Welcome to Gadgets, they're tiny bits of fun for your computer. What do you want to do?"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "WELCOME_ADD_GADGET");
xmlTextWriterWriteString (writer, _("Add a new Gadget"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "WELCOME_CHECK_UPDATES");
xmlTextWriterWriteString (writer, _("Check for updates"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "WELCOME_NO_UPDATES");
xmlTextWriterWriteString (writer, _("Sorry, we don't have any updated Gadgets available. You could:"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "PLUGIN_DATA_BULLET");
xmlTextWriterWriteString (writer, _("» "));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "PLUGIN_DATA_SEPARATOR");
xmlTextWriterWriteString (writer, _(" / "));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "PLUGIN_VERSION");
xmlTextWriterWriteString (writer, _("Version "));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "PLUGIN_KILOBYTES");
xmlTextWriterWriteString (writer, _(" KB"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_ALL");
xmlTextWriterWriteString (writer, _("All languages"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_BG");
xmlTextWriterWriteString (writer, _("Bulgarian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_CA");
xmlTextWriterWriteString (writer, _("Catalan"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_CS");
xmlTextWriterWriteString (writer, _("Czech"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_DA");
xmlTextWriterWriteString (writer, _("Danish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_DE");
xmlTextWriterWriteString (writer, _("German"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_EL");
xmlTextWriterWriteString (writer, _("Greek"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_EN");
xmlTextWriterWriteString (writer, _("English (US)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_EN_GB");
xmlTextWriterWriteString (writer, _("English (UK)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_ES");
xmlTextWriterWriteString (writer, _("Spanish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_FI");
xmlTextWriterWriteString (writer, _("Finnish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_FR");
xmlTextWriterWriteString (writer, _("French"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_HI");
xmlTextWriterWriteString (writer, _("Hindi"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_HR");
xmlTextWriterWriteString (writer, _("Croatian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_HU");
xmlTextWriterWriteString (writer, _("Hungarian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_ID");
xmlTextWriterWriteString (writer, _("Indonesian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_IT");
xmlTextWriterWriteString (writer, _("Italian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_JA");
xmlTextWriterWriteString (writer, _("Japanese"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_KO");
xmlTextWriterWriteString (writer, _("Korean"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_NL");
xmlTextWriterWriteString (writer, _("Dutch"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_NO");
xmlTextWriterWriteString (writer, _("Norwegian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_PL");
xmlTextWriterWriteString (writer, _("Polish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_PT_BR");
xmlTextWriterWriteString (writer, _("Portuguese (Brazil)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_PT_PT");
xmlTextWriterWriteString (writer, _("Portuguese (Portugal)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_RO");
xmlTextWriterWriteString (writer, _("Romanian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_RU");
xmlTextWriterWriteString (writer, _("Russian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_SK");
xmlTextWriterWriteString (writer, _("Slovak"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_SL");
xmlTextWriterWriteString (writer, _("Slovenian"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_SV");
xmlTextWriterWriteString (writer, _("Swedish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_TH");
xmlTextWriterWriteString (writer, _("Thai"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_TR");
xmlTextWriterWriteString (writer, _("Turkish"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_ZH_CN");
xmlTextWriterWriteString (writer, _("Chinese (Simplified)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "LANGUAGE_ZH_TW");
xmlTextWriterWriteString (writer, _("Chinese (Traditional)"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_ALL");
xmlTextWriterWriteString (writer, _("All"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_NEW");
xmlTextWriterWriteString (writer, _("New"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_RECOMMENDATIONS");
xmlTextWriterWriteString (writer, _("Recommendations"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_GOOGLE");
xmlTextWriterWriteString (writer, _("Google Created"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_RECENTLY_USED");
xmlTextWriterWriteString (writer, _("Recently Used"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_UPDATES");
xmlTextWriterWriteString (writer, _("Updates"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_NEWS");
xmlTextWriterWriteString (writer, _("News"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_SPORTS");
xmlTextWriterWriteString (writer, _("Sports"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_LIFESTYLE");
xmlTextWriterWriteString (writer, _("Lifestyle"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_FINANCE");
xmlTextWriterWriteString (writer, _("Finance"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_TOOLS");
xmlTextWriterWriteString (writer, _("Tools"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_FUN_GAMES");
xmlTextWriterWriteString (writer, _("Fun and Games"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_TECHNOLOGY");
xmlTextWriterWriteString (writer, _("Technology"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_COMMUNICATION");
xmlTextWriterWriteString (writer, _("Communication"));
xmlTextWriterEndElement (writer);
xmlTextWriterStartElement (writer, "CATEGORY_HOLIDAY");
xmlTextWriterWriteString (writer, _("Holidays"));
xmlTextWriterEndElement (writer);
 
  xmlTextWriterEndElement (writer); /* close strings */ 
  xmlTextWriterEndDocument(writer); 

  return 0; 
} 
