#include <stdio.h>
#include <libxml/xmlreader.h>


const char* writer_beginning = " \
#include \"config.h\" \n\
\n\
#include <locale.h> \n\
#include <stdio.h> \n                           \
#include <string.h> \n\
#include <libxml/encoding.h> \n\
#include <libxml/xmlwriter.h> \n\
#include <glib/gi18n.h> \n\
\n\
int \n\
main (int argc, char* argv[]) \n\
{ \n\
  setlocale (LC_ALL, argv[2]); \n\
  bindtextdomain (GETTEXT_PACKAGE, \"locale\");\n\
  bind_textdomain_codeset (GETTEXT_PACKAGE, \"UTF-8\");\n\
  textdomain (GETTEXT_PACKAGE); \n\
\n\
  xmlTextWriterPtr writer; \n                   \
  xmlChar *tmp; \n\
\n\
  writer = xmlNewTextWriterFilename (argv[1], 0); \n\
  xmlTextWriterStartDocument (writer, NULL, \"utf-8\", NULL); \n\
  xmlTextWriterStartElement (writer, \"strings\"); \n\
";

const char* writer_end = " \n\
  xmlTextWriterEndElement (writer); /* close strings */ \n\
  xmlTextWriterEndDocument(writer); \n\
\n\
  return 0; \n\
} \n\
";

FILE* output;

static void
on_node(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
	name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

    if (xmlTextReaderDepth (reader) == 1 &&
        xmlTextReaderNodeType (reader) == 1) {

      fprintf (output, "xmlTextWriterStartElement (writer, \"%s\");\n",
               name);

      return;
    }

    if (xmlTextReaderDepth (reader) == 2 &&
        xmlTextReaderNodeType (reader) == 3) {

      fprintf (output, "xmlTextWriterWriteString (writer, _(\"");
      char* p;
      for (p = value; *p; p++) {
        if (*p == '\n')
          fputs ("&#10;", output);
        else
          fputc (*p, output);
      }
      fprintf (output, "\"));\n");
      fprintf (output, "xmlTextWriterEndElement (writer);\n");

    }
    return;
}

int
main (int argc, char* argv[])
{
  xmlTextReaderPtr reader;
  int ret;
  int exitcode = 0;

  reader = xmlReaderForFile(argv[1], NULL, 0);
  output = fopen (argv[2], "w");
  if (!reader) {
    fprintf(stderr, "Unable to open %s\n", argv[1]);
    return 1;
  }

  ret = xmlTextReaderRead(reader);
  fputs (writer_beginning, output);
  while (ret == 1) {
    on_node (reader);
    ret = xmlTextReaderRead(reader);
  }
  xmlFreeTextReader(reader);
  fputs (writer_end, output);
  if (ret != 0) {
    fprintf(stderr, "%s : failed to parse\n", argv[1]);
    exitcode = 2;
  }
  fclose (output);
  return exitcode;
}
