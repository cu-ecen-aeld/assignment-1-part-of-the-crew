#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <errno.h>

int main (int argc, char* argv[])
{
  //const char* program_name = argv[0];
  openlog("Logs", LOG_PID, LOG_USER);
  //syslog (LOG_INFO, "Program started by User %d", getuid ());

  if (1 == argc)
  {
    syslog (LOG_ERR, "No the first argument(writefile)");
    goto err__;
  }
  if (2 == argc)
  {
    syslog (LOG_ERR, "No the second argument(writestr)");
    goto err__;
  }
  const char* file_path = argv[1];
  const char* file_text = argv[2];

//void openlog(const char *ident, int option, int facility);
//void syslog(int priority, const char *format, ...);



  FILE *const f_output = fopen(file_path, "a");
  if (NULL == f_output)
  {
    syslog (LOG_ERR, "%s", strerror(errno));
    goto err__;
  }


  syslog (LOG_DEBUG, "Writing %s to %s", file_text, file_path);

  if (fprintf(f_output,"%s\n", file_text) < 0)
      syslog (LOG_ERR, "%s", strerror(errno));

  fclose(f_output);
  closelog();
  return EXIT_SUCCESS;

  err__:
  closelog();
  return EXIT_FAILURE;
}
