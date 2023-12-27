#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <errno.h>
#include <err.h>

#include <stdlib.h>
#include <signal.h>

#include <ctype.h>
#include <inttypes.h>

#include <syslog.h>

#include <sys/types.h>
#include <netdb.h>

#include <sys/stat.h>

#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include <sys/queue.h>
#include <stdatomic.h>

#define USE_AESD_CHAR_DEVICE 1


typedef struct
{
  pthread_t thread;
  int cs_e;
  int cs;
  struct sockaddr_storage their_addr;
  char *wr_buf;
} desc_list_t;

// SLIST.
typedef struct slist_data_s slist_data_t;
struct slist_data_s {
    desc_list_t desc_list;
    SLIST_ENTRY(slist_data_s) entries;
};

typedef struct
{
  int server_fd_e;
  int server_fd;
  int f_output_e;
  int f_output;
  struct addrinfo *res;
  const char* file_path;
  int demonize;
  pthread_mutex_t mx;
  pthread_t thread_time;
  int cancel_threads;
  SLIST_HEAD(slisthead, slist_data_s) head;
} desc_t;

//desc_t desc = {.file_path = "./aesdsocketdata"};
#ifdef USE_AESD_CHAR_DEVICE
    desc_t desc = {.file_path = "/dev/aesdchar"};
#else
    desc_t desc = {.file_path = "/var/tmp/aesdsocketdata"};
#endif
    
desc_t *desc_p = &desc;

void uninit (int sig_num)
{
  desc_p->cancel_threads = 1;
  //if (desc_p->res)
  //  freeaddrinfo(desc_p->res);           /* No longer needed */

  if (desc_p->server_fd_e)
  {
    shutdown(desc_p->server_fd, SHUT_RDWR);
    close(desc_p->server_fd);
  }


  // Read2 (remove).
  while (!SLIST_EMPTY(&desc.head)) {
    slist_data_t *datap = SLIST_FIRST(&desc.head);

    if (0 == desc.demonize) printf("free: %p, cs = %d\n", datap, datap->desc_list.cs);

    //if (datap->desc_list.cs_e)
    //{
      //pthread_kill(datap->desc_list.thread, SIGRTMIN+3);
      pthread_cancel(datap->desc_list.thread);
      pthread_join(datap->desc_list.thread, NULL);
      shutdown(datap->desc_list.cs, SHUT_RDWR);
      close(datap->desc_list.cs);
    //}
    free(datap->desc_list.wr_buf);

    SLIST_REMOVE_HEAD(&desc.head, entries);
    free(datap);
  }

  //pthread_kill(desc_p->thread_time, SIGRTMIN+3);

  pthread_join(desc_p->thread_time, NULL);
  pthread_cancel(desc_p->thread_time);
  pthread_mutex_unlock(&desc_p->mx);


  if (desc_p->f_output_e)
  {
#ifdef USE_AESD_CHAR_DEVICE
    remove(desc_p->file_path);
#endif
    close(desc_p->f_output);
  }
  syslog (LOG_INFO, "Caught signal, exiting");
  closelog();

  pthread_mutex_destroy(&desc_p->mx);

  /*
    // Read1.
    SLIST_FOREACH(datap, &head, entries) {
      printf("Read1: %d\n", datap->value);
    }
    printf("\n");

    // Read2 (remove).
    while (!SLIST_EMPTY(&head)) {
      datap = SLIST_FIRST(&head);
      printf("Read2: %d\n", datap->value);
      SLIST_REMOVE_HEAD(&head, entries);
      free(datap);
    }
  */

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

void mask_sig(void)
{
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN+3);

  pthread_sigmask(SIG_BLOCK, &mask, NULL);
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

void* threadfunc_time(void* thread_param)
{
  char outstr[200];
  time_t t;
  struct tm *tmp;
  int ssize = 0;
  //time_t start_time = time(NULL);
  mask_sig();
  while (0 == desc_p->cancel_threads)
  {
/*
    usleep(100);
    time_t current_time = time(NULL);
    if(difftime(current_time, start_time) < 10)
    {
        continue;
    }
*/
    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
      if (0 == desc.demonize) printf("localtime \n");
    }

    if ((ssize = strftime(outstr, sizeof(outstr), "timestamp:%a, %d %b %Y %T %z\n", tmp)) == 0) {
        if (0 == desc.demonize) printf("strftime returned 0");
    }

    pthread_mutex_lock(&desc_p->mx);
    write(desc.f_output, outstr, ssize);
    pthread_mutex_unlock(&desc_p->mx);

    //if (0 == desc.demonize) printf("%s", outstr);
    sleep(10);

  }

  pthread_exit(NULL);
}


void* threadfunc(void* thread_param)
{
  mask_sig();
  //if (0 == desc.demonize) printf("Thread created\n");
  slist_data_t* arg = (slist_data_t *) thread_param;
  desc_list_t* par = &arg->desc_list;
/*
  const struct timespec tto = {.tv_sec     = thread_func_args->wait_to_obtain_ms / 1000,
                               .tv_nsec = (thread_func_args->wait_to_obtain_ms % 1000) * 1000000};
  const struct timespec ttr = {.tv_sec     = thread_func_args->wait_to_release_ms / 1000,
                               .tv_nsec = (thread_func_args->wait_to_release_ms % 1000) * 1000000};
*/






  //socklen_t len = sizeof addr;
  struct sockaddr_storage addr = par->their_addr;
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

  if (0 == desc.demonize)
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
  memset (buf , 0 , SIZEBUF);
  while ((ssize = read(par->cs, buf, SIZEBUF)) > 0)
  {
    for (int i = 0; i < ssize ; i++)
    {
      if ('\n' == buf[i])
      {
        ssize = i + 1;
        pthread_mutex_lock(&desc_p->mx);
        write(desc.f_output, buf, ssize);
        pthread_mutex_unlock(&desc_p->mx);
        //printf ("2Printing in file: %s )\n", buf);
        goto note_a;
      }
    }
    pthread_mutex_lock(&desc_p->mx);
    write(desc.f_output, buf, ssize);
    pthread_mutex_unlock(&desc_p->mx);
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
  pthread_mutex_lock(&desc_p->mx);
  if ( -1 == fstat(desc.f_output, &st))
    if (0 == desc.demonize) printf("Problem to stat \n");
  //pthread_mutex_unlock(&desc_p->mx);

  par->wr_buf = calloc(sizeof(char) * st.st_size, 1);

  //pthread_mutex_lock(&desc_p->mx);
  lseek(desc.f_output, 0, SEEK_SET);
  if ((ssize = read(desc.f_output, par->wr_buf, st.st_size)) < 0)
  {
    if (0 == desc.demonize) printf("Problem to read file \n");
  }
  pthread_mutex_unlock(&desc_p->mx);
  /*
  for (int i = 0; i < st.st_size - 1; i++)
    printf("%s", wr_buf);
  printf("\nEND\n");
  */

  if ( -1 == sendall(par->cs, par->wr_buf, (int*)&st.st_size))
    if (0 == desc.demonize) printf("Problem to send \n");

  if (0 == desc.demonize) printf("Send...%ld bytes\n", st.st_size);

  par->cs_e = 0;
/*
  free(par->wr_buf);
  shutdown(par->cs, SHUT_RDWR);
  close(par->cs);
*/
  syslog (LOG_INFO, "Closed connection from %s", ipstr);


  pthread_exit(NULL);
  //return thread_param;
}

int main (int argc, char *argv[])
{
  int opt = 0;
  const char ApOptions [] = "dD";
  char *program_name = argv[0];
  int status = 0;

  if (argc > 0) {
    printf(" Program started :: %s\n", program_name);

    while ((opt = getopt (argc, argv, ApOptions)) != -1) {
      switch (tolower(opt)) {
        case 'd':
          desc.demonize = 1;
          printf( "demonization...\n");
          break;
        case '?':
          printf( "  Usage temporary unavailabled\n");
          break;
      }
    }
  }


terminationSignal(uninit);

pthread_mutex_init(&desc_p->mx, NULL);
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

freeaddrinfo(desc.res);
desc.res = NULL;

//pid_t child;
if (1 == desc.demonize)
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
/*
      if (open("/dev/null",O_RDONLY) == -1) {
        syslog (LOG_INFO, "failed to reopen stdin");
      }
      if (open("/dev/null",O_WRONLY) == -1) {
        syslog (LOG_INFO, "failed to reopen stdout");
      }
      if (open("/dev/null",O_RDWR) == -1) {
        syslog (LOG_INFO, "failed to reopen stderr");
      }
*/
  /* child process */
      //int ret = execv(command[0], command);
      //printf("I'm the child ret = %d \n", ret);

  } else {

    exit(EXIT_SUCCESS);
  }
}

desc.demonize = 1;

status = listen(desc.server_fd, 10);
if (0 != status){
  if (0 == desc.demonize) printf("can't listen net socket, errno = %d( %s )\n", errno, strerror( errno ));
  uninit (-1);
}


desc.f_output = open(desc.file_path, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
if (-1 == desc.f_output)
{
  if (0 == desc.demonize) printf("Cannot open the file %s\n", desc.file_path);
  uninit (-1);
}
desc.f_output_e = 1;


#ifdef USE_AESD_CHAR_DEVICE
status = pthread_create( &desc.thread_time, NULL, threadfunc_time, desc_p );
if (0 != status){
  if (0 == desc.demonize) printf("can't create TIME thread status = %d\n", status);
  uninit (-1);
}
#endif





SLIST_INIT(&desc.head);
slist_data_t *datap=NULL;


int cs = 0;
int cs_e = 0;


struct sockaddr_storage their_addr;
socklen_t addr_size = sizeof their_addr;

while(1)
{
  if (0 == desc.demonize) printf("listen...\n\n");
  cs_e++;
  cs = accept(desc.server_fd, (struct sockaddr *)&their_addr, &addr_size);
  if (0 > cs){
    if (0 == desc.demonize) printf("can't accept net socket, errno = %d( %s )\n", errno, strerror( errno ));
    uninit (-1);
  }

  // Write to slist
  datap = calloc(sizeof(slist_data_t), 1);
  if (0 == desc.demonize) printf("calloc: %p, cs = %d\n", datap, cs);
  datap->desc_list.cs = cs;
  datap->desc_list.cs_e = cs_e;
  datap->desc_list.their_addr = their_addr;
  if (0 == desc.demonize) printf("Insert cs: %d\n", cs);

  SLIST_INSERT_HEAD(&desc.head, datap, entries);


  status = pthread_create( &datap->desc_list.thread, NULL, threadfunc, datap );
  if (0 != status){
    if (0 == desc.demonize) printf("can't create thread status = %d\n", status);
    uninit (-1);
  }

  /*
  if (0 == desc.demonize) printf("check slist-------------------\n");
  int j = 0;
  SLIST_FOREACH(datap, &desc.head, entries)
  {
    if (0 == desc.demonize) printf("iteration %d\n", j);
    j++;
    if (0 == datap->desc_list.cs_e)
    {
      if (0 == desc.demonize) printf("freed cs = %d\n", datap->desc_list.cs);
      shutdown(datap->desc_list.cs, SHUT_RDWR);
      close(datap->desc_list.cs);
      free(datap->desc_list.wr_buf);
      pthread_join(datap->desc_list.thread, NULL);

      SLIST_REMOVE(&desc.head, datap, slist_data_s, entries);
      free(datap);
    }

  }
*/
/*
  // Read2 (remove).
  while (!SLIST_EMPTY(&desc.head)) {
    slist_data_t *datap_ = SLIST_FIRST(&desc.head);

    printf("desc_list.cs: %d\n", datap_->desc_list.cs);




    if (0 == datap_->desc_list.cs_e)
    {
      if (0 == desc.demonize) printf("freed cs = %d\n", datap_->desc_list.cs);
      shutdown(datap_->desc_list.cs, SHUT_RDWR);
      close(datap_->desc_list.cs);
      free(datap_->desc_list.wr_buf);
      pthread_join(datap_->desc_list.thread, NULL);

      SLIST_REMOVE_HEAD(&desc.head, entries);
      free(datap_);
    }
  }
*/
} //while

}


