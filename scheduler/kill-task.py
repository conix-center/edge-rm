#!/usr/bin/env python3
import sys
import argparse
import pydig

from scheduler_library import Framework

def main(host, port, taskId, all):  # pragma: no cover
    framework = Framework("Kill Framework", host, port)
    if all == True:
    	framework.killAllTasks()
    else:
    	framework.killTask(taskId)

if __name__ == '__main__':  # pragma: no cover
    parser = argparse.ArgumentParser(description='Launch a CoAP Resource Manager Framework')
    parser.add_argument('--host', required=True, help='the Edge RM Master IP to register with.')
    parser.add_argument('--port', required=False, default=5683, help='the Edge RM Master port to register on.')
    parser.add_argument('--taskid', required = False, help='Value of task to kill')
    parser.add_argument('--all', action='store_true', help="Specify that all tasks should be killed")
    args = parser.parse_args()
    main(args.host, args.port, args.taskid, args.all)
