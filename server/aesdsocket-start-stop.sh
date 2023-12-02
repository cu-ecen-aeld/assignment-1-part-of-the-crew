#!/bin/sh
# Author: NDunichev


# Get function from functions library
#. /etc/init.d/functions

NAME=aesdsocket
# Start the service
start() {
        #initlog -c "echo -n Starting name server: "
        echo "$NAME starting..."
        start-stop-daemon -S -n $NAME -a /usr/bin/$NAME -- -d
        #start-stop-daemon -S -n $NAME -a /home/amente/Downloads/EmLinuxCourseAssigments/A1/server/$NAME -- -d
        ### Create the lock file ###
        touch /var/lock/subsys/$NAME
        #success $"$name server startup"
        echo "$NAME started"
}

# stop the service
stop() {
        #initlog -c "echo -n Stopping FOO server: "
        #killproc FOO
        start-stop-daemon -K -n $NAME
        ### Now, delete the lock file ###
        rm -f /var/lock/subsys/$NAME
        echo "$NAME stopped"
}

### main logic ###
case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  status)
        #status FOO
        ;;
  restart|reload|condrestart)
        stop
        start
        ;;
  *)
        echo $"Usage: $0 {start|stop|restart|reload|status}"
        exit 1
esac

exit 0
