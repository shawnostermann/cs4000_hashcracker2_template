/* File:     hash_cracker.c
 * Ostermann's example solution
 * Mar 24, 2022
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <mpi.h>
#include <sys/wait.h>


//
// You will need to add this to the top of your hash_cracker.c file:
char *MakeSha256( char *salt,  char *pwd_guess);
//
//




// from <util.h>
int openpty(int *aprimary, int *areplica, char *name, void *termp, void *winp);

static int ssl_debug = 0;  // ONLY these routines

static int pid_openssl = -1;

static void TerminateOpenssl(void) {
   if (ssl_debug)
      fprintf(stderr,"Killing openssl child %d\n", pid_openssl);
   kill(pid_openssl,9);
}

static void wakeup(int sig)
{
    printf(" <<timeout>>\n");
    fflush(stdout);
}


// write the stupid popen() call by hand so I can see what it's doing...
static int popen_openssl(char *salt, int *pfd_primary) {
   int fd_replica = -1;

   if (ssl_debug>2) {
      printf("fd_primary = %d\n", *pfd_primary);
      printf("fd_replica = %d\n", fd_replica);
   }


   if (openpty(pfd_primary, &fd_replica, NULL, NULL, NULL) == -1) {
      perror("openpty");
      exit(-99);
   }

   if (ssl_debug>2) {
      printf("fd_primary = %d\n", *pfd_primary);
      printf("fd_replica = %d\n", fd_replica);
   }

   if ((pid_openssl=fork()) == 0) {
      dup2(fd_replica,0);
      dup2(fd_replica,1);
      close(fd_replica);
      close(fd_replica);

      // put this into MY environment - it will get inherited by the BASH program I'm about to start
      // that will cause BASH to exit if no input flows for 60 seconds
      // hopefully keeping openssl programs from sitting around if something crashes
      putenv("TMOUT=60");

      // kill all instances of "openssl" that I might currently have running on this computer 
      // to help keep them from building up and taking ptys
      system("killall -q openssl");
      
      char ssl_command[1024];
      // snprintf(ssl_command, sizeof(ssl_command), "tee test.in.log | cat | tee test.out.log\n");
      // snprintf(ssl_command, sizeof(ssl_command), "openssl passwd -5 -salt '%s' -stdin 2>&1 | tee test.out.log\n", salt);
      // adds a 30 second Idle Time timeout just in case so we don't leave junk around
      snprintf(ssl_command, sizeof(ssl_command), 
           " openssl passwd -5 -salt %s -stdin\n", salt);
      execl("/bin/bash","bash","-c",ssl_command,NULL);
      perror("/bin/bash");
      exit(-10);
   }

   close(fd_replica);

   // bug fix - clean up any zombies
   if (ssl_debug>2) printf("popen_openssl: waiting for children...\n");
   while(1) {
      int pid = waitpid((pid_t)-1, NULL, WNOHANG);
      if (pid > 0) {
         if (ssl_debug>2) 
            printf("popen_openssl: child process %d is dead\n", pid);
      } else if (pid ==0)
         break;
   }
   if (ssl_debug>2) printf("popen_openssl: all zombies shot in the head\n");

   // Install alarm clock signal handler
   /* call 'wakeup()' when the alarm goes off */
   {
	struct sigaction sa;

	memset(&sa,0,sizeof(sa));
	sa.sa_handler = wakeup;
	sa.sa_flags = 0;	/* always interrupt system calls */
	sigaction(SIGALRM,&sa,NULL);
   }

   atexit(TerminateOpenssl);  // make sure the pseudotty gets closed with openssl...
   
   return(1);
}

// You shouldn't need to change anything about this routine
char *MakeSha256( char *salt,  char *pwd_guess) {
   char command[1024];
   char result[100];
   int ret;
   static int fd_from_ssl = -1;
   char *ptr = NULL;
   int retries = 0;

   if (fd_from_ssl < 0) {
      if (fd_from_ssl < -1) fprintf(stderr,"Re-Opening pty\n");
      popen_openssl(salt, &fd_from_ssl);
   }

   // now, the command is running and waiting to read passwords from its stdin
   sprintf(command,"%s\n", pwd_guess);
   if (ssl_debug>1) fprintf(stderr,"Sending to openssl on fd %d: '%s'\n", fd_from_ssl, command);
   if ((ret = write(fd_from_ssl,command,strlen(command))) == -1) {
      perror("openssl write");
      exit(-2);
   }
   if (ssl_debug>1) fprintf(stderr,"openssl: sent %d bytes: '%s'\n", ret, pwd_guess);

   while (++retries) {
      int len;

      // get the hash back
      if (ssl_debug>1) fprintf(stderr,"Waiting for openssl answer on fd %d...\n", fd_from_ssl);
      alarm(5);   // give up after 5 seconds
      if ((len=read(fd_from_ssl,result, sizeof(result))) == -1) {
         alarm(0);
         if (errno == EINTR) {
               fprintf(stderr,"Gave up waiting for OpenSSL answer\n");
               exit(-42);
         }
         perror("openssl read");
         exit(-3);
      }
      alarm(0);  // turn off alarm clock signal

      result[len-1] = '\00'; // make sure it's NULL terminated

      if (ssl_debug>1) fprintf(stderr,"OpenSSL answer Digest: '%s'", result);
      ptr = result;
      ptr=index(result+1,'$');   // find the second $
      if (!ptr) {
         if (ssl_debug>1)
            fprintf(stderr,"Malformed 1 openssl answer: '%s'\n", result);
         if (retries > 10) {
            fprintf(stderr,"Too many retries waiting for OpenSSL answer\n");
            exit(-43);
         }
         continue;
      }
      ptr=1+index(ptr+1,'$');   // find the third $
      if (!ptr) {
         fprintf(stderr,"Malformed 2 openssl answer: '%s'\n", result);
         exit(-6);
      }
      if (strlen(ptr) < 40) {  // might have newline... dunno ...
         fprintf(stderr,"Malformed 3 openssl answer: (len:%ld) '%s'\n", strlen(ptr),result);
         exit(-7);
      }
      if (ssl_debug>1) printf("OpenSSL Hash: '%s'", ptr);
      break;
   }

   return(strdup(ptr));
}
