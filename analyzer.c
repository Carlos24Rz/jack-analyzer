#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// POSIX
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>

#include "parser.h"

#define JACK_XML_EXTENSION "xml"
#define JACK_FILE_EXTENSION ".jack"
#define MAX_FILENAME_LENGTH 256

bool analyze_file(const char *jack_file)
{
  int i = 0;
  const char *current_char = jack_file;
  char input_filename[MAX_FILENAME_LENGTH + 1];
  char xml_filename[MAX_FILENAME_LENGTH + 1];
  const char *extension = strrchr(jack_file, '.');

  FILE *ast_stream, *xml_out;
  char *ast_buf;
  size_t ast_size;
  Parser *parser;
  bool ret;

  while (current_char != extension)
  {
    input_filename[i++] = *current_char;

    current_char++;
  }

  input_filename[i] = '\0';

  parser = init_parser(jack_file);

  if (parser == NULL)
  {
    fprintf(stderr, "Fail to initialize parser for file %s\n", jack_file);
    return false;
  }

  ast_stream = open_memstream(&ast_buf, &ast_size);

  if (ast_stream == NULL)
  {
    fprintf(stderr, "Fail to create buffer for AST: %s\n", strerror(errno));
    fini_parser(parser);
    return false;
  }

  // Parse file
  ret = compileClass(parser, ast_stream);

  fclose(ast_stream);

  if (!ret)
  {
    fprintf(stderr, "Fail to parse file %s\n", jack_file);
    free(ast_buf);
    fini_parser(parser);
    return false;
  }

  // Create output xml file
  snprintf(xml_filename, sizeof(xml_filename), "%s.%s", input_filename, JACK_XML_EXTENSION);

  xml_out = fopen(xml_filename, "w");

  if (xml_out == NULL)
  {
    fprintf(stderr, "Fail to create xml file %s: %s", xml_filename, strerror(errno));
    free(ast_buf);
    fini_parser(parser);
    return false;
  }

  fwrite(ast_buf, sizeof(char), ast_size, xml_out);
  fclose(xml_out);
  free(ast_buf);
  fini_parser(parser);

  return true;
}

bool is_file_jack(const char *filename)
{
  char *file_extension = strrchr(filename, '.');

  if (file_extension == NULL)
    return false;

  return strcmp(file_extension, JACK_FILE_EXTENSION) == 0;
}

bool analyze_dir()
{
  DIR *directory = opendir(".");
  struct dirent *dir_entry;
  int total_jack_files = 0;
  int succ_jack_files = 0;

  if (directory == NULL)
  {
    fprintf(stderr, "Failed to open directory\n");
    return false;
  }

  while ((dir_entry = readdir(directory)) != NULL)
  {
    if (dir_entry->d_type != DT_REG)
      continue;

    if (!is_file_jack(dir_entry->d_name))
      continue;

    total_jack_files++;

    if (analyze_file(dir_entry->d_name))
      succ_jack_files++;
  }

  if (total_jack_files == 0)
  {
    fprintf(stderr, "No jack files found in directory\n");
  }
  else
  {
    fprintf(stderr, "Parsed %d out of %d files\n", succ_jack_files, total_jack_files);
  }

  return total_jack_files == succ_jack_files;
}

int main(int argc, char *argv[])
{
  struct stat input_path_stat;

  if (argc > 2)
  {
    fprintf(stderr, "Usage: ./parser [filename | directory]\n");
    return 1;
  }

  if (argc == 2)
  {
    if (stat(argv[1], &input_path_stat) != 0)
    {
      fprintf(stderr, "Failed to open %s: %s\n", argv[1], strerror(errno));
      return 1;
    }

    if (S_ISREG(input_path_stat.st_mode))
    {
      // Check if file is jack
      char file_path[MAX_FILENAME_LENGTH + 1];
      char dir_path[MAX_FILENAME_LENGTH + 1];
      char *file_name;
      char *dir_name;

      strcpy(file_path, argv[1]);

      file_name = basename(file_path);

      if (!is_file_jack(file_name))
      {
        fprintf(stderr, "Invalid file %s: Must provide a valid .jack file\n", file_name);
        return 1;
      }

      strcpy(dir_path, argv[1]);

      dir_name = dirname(dir_path);

      // Switch to directory
      if (chdir(dir_name) != 0)
      {
        fprintf(stderr, "Failed to open directory %s: %s\n", dir_name, strerror(errno));
        return 1;
      }

      if (analyze_file(file_name) == false)
        return 1;
    }
    else if (S_ISDIR(input_path_stat.st_mode))
    {
      // Switch to directory
      if (chdir(argv[1]) != 0)
      {
        fprintf(stderr, "Failed to open directory %s: %s\n", argv[1], strerror(errno));
        return 1;
      }

      if (analyze_dir() == false)
        return 1;
    }
  }
  else if (analyze_dir() == false)
  {
    return 1;
  }

  return 0;
}