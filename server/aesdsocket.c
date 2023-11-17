#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#include <errno.h>
#include <err.h>

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

#include <ctype.h>
#include <inttypes.h>


#include <syslog.h>

#include <sys/types.h>
#include <netdb.h>

#include <sys/stat.h>

#include <arpa/inet.h>

typedef struct
{
  int server_fd;
  int cs;
  int f_output;
  int server_fd_e;
  int cs_e;
  int f_output_e;
  struct addrinfo *res;
  const char* file_path;
  char file_path_new[50];

} desc_t;

//desc_t desc = {.file_path = "./aesdsocketdata"};
desc_t desc = {.file_path = "/var/tmp/aesdsocketdata"};
desc_t *desc_p = &desc;

void uninit (int sig_num)
{
  if (desc_p->res)
    freeaddrinfo(desc_p->res);           /* No longer needed */

  if (desc_p->server_fd_e)
  {
    shutdown(desc_p->server_fd, 2);
    close(desc_p->server_fd);
  }

  if (desc_p->cs_e)
  {
    shutdown(desc_p->cs, 2);
    close(desc_p->cs);
  }

  syslog (LOG_INFO, "Caught signal, exiting");
  closelog();


  if (desc_p->f_output_e)
  {
    remove(desc.file_path_new);
    close(desc_p->f_output);
  }


  exit (sig_num);
}

void terminationSignal(void (*handler)(int)) {
  struct sigaction action;
  const int tSignals[] = {SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGBUS,
                          SIGSEGV, SIGSYS, SIGPIPE, SIGTERM, SIGPWR};
  const size_t sigCount = sizeof(tSignals) / sizeof(int);

  action.sa_handler = handler;
  action.sa_flags = 0;

  for (size_t i = 0; i < sigCount; ++i) {
    sigemptyset(&action.sa_mask);
    for (size_t j = 0; j < sigCount; ++j) {
      if (i != j) {
        sigaddset(&action.sa_mask, tSignals[j]);
      }
    }
    sigaction(tSignals[i], &action, NULL);
  }
}


int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}


int main (int argc, char *argv[])
{
  int opt = 0;
  const char ApOptions [] = "dD";
  char *program_name = argv[0];
  int demonize = 0;
  int status = 0;

  if (argc > 0) {
    printf(" Program started :: %s\n", program_name);

    while ((opt = getopt (argc, argv, ApOptions)) != -1) {
      switch (tolower(opt)) {
        case 'd':
          demonize = 1;
          printf( "demonization...\n");
          break;
        case '?':
          printf( "  Usage temporary unavailabled\n");
          break;
      }
    }
  }


terminationSignal(uninit);


openlog("aesd", LOG_PID, LOG_USER);

struct addrinfo hints;
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_INET;//AF_UNSPEC;  // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
//htons ( 9000 )
status = getaddrinfo(NULL, "9000", &hints, &desc.res);
if (status != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    uninit(-1);
}


desc.server_fd = socket(desc.res->ai_family, desc.res->ai_socktype, desc.res->ai_protocol);
if (-1 == desc.server_fd){
  printf("can't create net socket? errno = %d(%s)\n", errno, strerror( errno ));
  uninit (-1);
}
desc.server_fd_e = 1;

int reuse = 1;
if (setsockopt(desc.server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
    printf("setsockopt(SO_REUSEADDR) failed");

if (setsockopt(desc.server_fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0)
    printf("setsockopt(SO_REUSEPORT) failed");


status = bind(desc.server_fd, desc.res->ai_addr, desc.res->ai_addrlen);
if (0 != status){
  printf("can't bind net socket, errno = %d( %s )\n", errno, strerror( errno ));
  uninit (-1);
}

//pid_t child;
if (1 == demonize)
{
  pid_t pid = fork();
  if (pid < 0)
  { /* error occurred */
      printf("Fork Failed\n");
      return EXIT_FAILURE;
  }

  if (pid == 0)
  {
      pid_t child;
      /* On success: The child process becomes session leader */
      if (setsid() < 0)
          exit(EXIT_FAILURE);

      //fork second time
      if ( (child = fork()) < 0) { //failed fork
          fprintf(stderr,"error: failed fork\n");
          exit(EXIT_FAILURE);
      }
      if( child > 0 ) { //parent
          printf("process_id of child process %d \n", child);
          exit(EXIT_SUCCESS);
      }
      //new file permissions
      umask(0);

      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      if ((chdir("/")) < 0)
        exit(EXIT_FAILURE);


  /* child process */
      //int ret = execv(command[0], command);
      //printf("I'm the child ret = %d \n", ret);

  } else {

    exit(EXIT_SUCCESS);
  }
}

status = listen(desc.server_fd, 2);
if (0 != status){
  if (0 == demonize) printf("can't listen net socket, errno = %d( %s )\n", errno, strerror( errno ));
  uninit (-1);
}


sprintf (desc.file_path_new, "%s_%ld", desc.file_path, 0L);

desc.f_output = open(desc.file_path_new, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO | S_IWOTH);
if (-1 == desc.f_output)
{
  if (0 == demonize) printf("Cannot open the file %s\n", desc.file_path_new);
  uninit (-1);
}

desc.f_output_e = 1;


while(1)
{

  if (0 == demonize) printf("listen...\n");


  struct sockaddr_storage their_addr;
  socklen_t addr_size = sizeof their_addr;

  desc.cs = accept(desc.server_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (0 > desc.cs){
    if (0 == demonize) printf("can't accept net socket, errno = %d( %s )\n", errno, strerror( errno ));
    uninit (-1);
  }
  desc.cs_e = 1;

  //socklen_t len = sizeof addr;
  struct sockaddr_storage addr = their_addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;


  //getpeername(desc.server_fd, (struct sockaddr*)&addr, &len);

  // deal with both IPv4 and IPv6:
  if (addr.ss_family == AF_INET) {
      struct sockaddr_in *s = (struct sockaddr_in *)&addr;
      port = ntohs(s->sin_port);
      inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
  } else { // AF_INET6
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
      port = ntohs(s->sin6_port);
      inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
  }

  if (0 == demonize)
  {
    printf("Peer IP address: %s\n", ipstr);
    printf("Peer port      : %d\n", port);
  }

  syslog (LOG_INFO, "Accepted connection from %s", ipstr);


  //new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
  /*
  char addr1[500];
  struct sockaddr_in peeraddr;
  //struct sockaddr addr;

  socklen_t serv_len = sizeof(addr);
  int serv_peer_err = getpeername(desc.cs, (struct sockaddr *)&addr, &serv_len);

  //socklen_t len = sizeof(addr);

  //getpeername(desc.cs, (struct sockaddr*)&addr1, 500);
  //memcpy(&peeraddr, addr1, sizeof peeraddr);
  //printf("Peer IP address: %s\n", inet_ntoa(addr1.sin_addr));
  //printf("Peer port      : %d\n", ntohs(addr1.sin_port));
  //printf("Peer port      : %s\n", inet_ntoa(peeraddr.sin_addr));

  // * getpeername
  struct sockaddr_in peeraddr;
  socklen_t peeraddrlen;

  status = getpeername(desc.cs, (struct sockaddr *) &peeraddr, &peeraddrlen);
  if (status == -1) {
    printf("can't getpeername net socket, errno = %d( %s )\n", errno, strerror( errno ));
    uninit (-1, desc_p);
  }

  char *peeraddrpresn = inet_ntoa(peeraddr.sin_addr);

  printf("Peer information:\n");
  printf("Peer Address Family: %d\n", peeraddr.sin_family);
  printf("Peer Port: %d\n", ntohs(peeraddr.sin_port));
  printf("Peer IP Address: %s\n\n", peeraddrpresn);

  //printf("Listening on ip %s and port %d\n", inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list), ntohs(serv_addr.sin_port));
  */
  //const char* file_path = "/var/tmp/aesdsocketdata";


  int ssize;
  const int SIZEBUF = 100;
  char buf[SIZEBUF];


  while ((ssize = read(desc.cs, buf, SIZEBUF)) > 0) {
      for (int i = 0; i < ssize ; i++)
        if ('\n' == buf[i])
        {
          ssize = i + 1;
          write(desc.f_output, buf, ssize);
          goto note_a;
        }
      write(desc.f_output, buf, ssize);
  }
  note_a:
  ;
  /*
  while ((ssize = read(desc.cs, buf, SIZEBUF)) > 0)
  {
    if (-1 == write(desc.f_output,buf[0], 1))
      printf ("Problem writing to the file, errno = %d( %s )\n", errno, strerror( errno ));
    i++;
    if ('\n' == buf[0])
      break;
  }
  */
  //printf("read file..., written to file bytes %d\n", i);

  struct stat st;
  if ( -1 == fstat(desc.f_output, &st))
    if (0 == demonize) printf("Problem to stat \n");

  char * wr_buf = calloc(sizeof(char) * st.st_size, 1);

  lseek(desc.f_output, 0, SEEK_SET);
  if ((ssize = read(desc.f_output, wr_buf, st.st_size)) < 0)
  {
    if (0 == demonize) printf("Problem to read file \n");
  }
  /*
  for (int i = 0; i < st.st_size - 1; i++)
    printf("%s", wr_buf);
  printf("\nEND\n");
  */

  if ( -1 == sendall(desc.cs, wr_buf, (int*)&st.st_size))
    if (0 == demonize) printf("Problem to send \n");

  if (0 == demonize) printf("Send...%ld bytes\n", st.st_size);


  free(wr_buf);
  shutdown(desc_p->cs, 2);
  close(desc_p->cs);

  syslog (LOG_INFO, "Closed connection from %s", ipstr);
}

}


