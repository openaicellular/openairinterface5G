"""
To create graphs and pickle from runtime statistics in L1,MAC,RRC,PDCP files
"""

import subprocess
import time
from datetime import datetime,timezone,timedelta
import shlex
import re
import sys
import pickle
import matplotlib.pyplot as plt
import numpy as np
import yaml
import os
import copy


class StatMonitor():
    def __init__(self,cfg_file):
        with open(cfg_file,'r') as file:
            self.d = yaml.load(file)
        for node in self.d:#so far we have enb or gnb as nodes
            for metric_l1 in self.d[node]: #first level of metric keys
                if metric_l1=="ue": #graph is a reserved word to configure graph paging, so it is disregarded
                    if self.d[node][metric_l1] is None:#first level is None -> create array
                        self.d[node][metric_l1]=[]
                    else: #first level is not None -> there is a second level -> create array
                        for metric_l2 in self.d[node][metric_l1]:
                            self.d[node][metric_l1][metric_l2]=[]                
            self.d[node]['rntis']={}

    def process_gnb (self,node_type,output):
        rnti='65535'
        bp_enable = False
        ts = time.gmtime(0) #Bad, but just creating a date
        for line in output:
            tmp=line.decode("utf-8")
            result=re.match(r'^.*\bUE RNTI ([a-zA-Z0-9]+) CU-UE-ID',tmp)
            if result is not None:
                rnti=result.group(1)
                if not rnti in self.d['gnb']['rntis']:
                   print("Found new RNTI: "+tmp)
                   self.d['gnb']['rntis'][rnti] = copy.deepcopy(self.d['gnb']['ue'])
                #bp_enable = True
                continue

            if bp_enable:
                print(tmp)
                breakpoint()
            result=re.match(r'^.*\bdlsch_rounds\b ([0-9]+)\/([0-9]+).*\bdlsch_errors\b ([0-9]+).*\bBLER\b ([0-9]+[.][0-9]+) \bMCS\b \([0-1]\) ([0-9]+)',tmp)
            if result is not None:
                self.d[node_type]['rntis'][rnti]['dlsch_err'].append(int(result.group(3)))
                try:
                   percentage=100.0*float(result.group(2))/float(result.group(1))
                except ZeroDivisionError:
                   percentage=0.0
                self.d[node_type]['rntis'][rnti]['dlsch_err_perc_round_1'].append(percentage)
                self.d[node_type]['rntis'][rnti]['dlbler'].append(float(result.group(4)))
                self.d[node_type]['rntis'][rnti]['dlmcs'].append(int(result.group(5)))
                result=re.match(r'^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}.\d{6}Z)',tmp)
                if result is not None:
                    ts = datetime.strptime(result.group(1),'%Y-%m-%d %H:%M:%S.%fZ')
                    if sys.version_info[0] == 3 and sys.version_info[1] < 11:
                       ts = ts.replace(tzinfo=timezone.utc)
                self.d[node_type]['rntis'][rnti]['timestamp'].append(ts)
                continue

            result=re.match(rf'^.*\b{rnti}\b: \bMAC\b.*\bTX\b +([0-9]+).*\bRX\b +([0-9]+)',tmp)
            if result is not None:
                if len(self.d[node_type]['rntis'][rnti]['dlsch_mbps']) == 0:
                   self.d[node_type]['rntis'][rnti]['dlsch_mbps'].append(0)
                   self.d[node_type]['rntis'][rnti]['ulsch_mbps'].append(0)
                   self.d[node_type]['rntis'][rnti]['dlsch_prev_bytes'].append(float(result.group(1)))
                   self.d[node_type]['rntis'][rnti]['ulsch_prev_bytes'].append(float(result.group(2)))
                else :
                   prev = self.d[node_type]['rntis'][rnti]['dlsch_prev_bytes'].pop()
                   #8 bits per byte, 10^20 Bits per Mbps, subtract bler
                   times =self.d[node_type]['rntis'][rnti]['timestamp'][-2:]
                   #delta_t = times[1]-times[0]
                   delta_t = timedelta(milliseconds=1280)
                   delta = (float(result.group(1)) - prev)*2**3/2**20*(1.0- self.d[node_type]['rntis'][rnti]['dlbler'][-1])/delta_t.total_seconds()
                   self.d[node_type]['rntis'][rnti]['dlsch_mbps'].append(delta)
                   self.d[node_type]['rntis'][rnti]['dlsch_prev_bytes'].append(float(result.group(1)))

                   prev = self.d[node_type]['rntis'][rnti]['ulsch_prev_bytes'].pop()
                   #8 bites per byte, 10^20 bits per mbps, subtract bler
                   delta = (float(result.group(2)) - prev)*2**3/2**20*(1.0-self.d[node_type]['rntis'][rnti]['ulbler'][-1])/delta_t.total_seconds()
                   self.d[node_type]['rntis'][rnti]['ulsch_mbps'].append(delta)
                   self.d[node_type]['rntis'][rnti]['ulsch_prev_bytes'].append(float(result.group(2)))
                continue

            result=re.match(r'^.*\bulsch_rounds\b ([0-9]+)\/([0-9]+).*\bulsch_errors\b ([0-9]+).*\bBLER\b ([0-9]+[.][0-9]+) \bMCS\b \([0-1]\) ([0-9]+)',tmp)
            if result is not None:
                self.d[node_type]['rntis'][rnti]['ulsch_err'].append(int(result.group(3)))
                try:
                   percentage=100.0*float(result.group(2))/float(result.group(1))
                except ZeroDivisionError:
                   percentage=0.0
                self.d[node_type]['rntis'][rnti]['ulsch_err_perc_round_1'].append(percentage)
                self.d[node_type]['rntis'][rnti]['ulbler'].append(float(result.group(4)))
                self.d[node_type]['rntis'][rnti]['ulmcs'].append(int(result.group(5)))
                continue

            for k in self.d[node_type]['rt']:
                result=re.match(rf'^.*\b{k}\b:\s+([0-9\.]+) us;\s+([0-9]+);\s+([0-9\.]+) us;',tmp)
                if result is not None:
                    self.d[node_type]['rt'][k].append(float(result.group(3)))


    def process_enb (self,node_type,output):
        for line in output:
            tmp=line.decode("utf-8")
            result=re.match(r'^.*\bPHR\b ([0-9]+).+\bbler\b ([0-9]+\.[0-9]+).+\bmcsoff\b ([0-9]+).+\bmcs\b ([0-9]+)',tmp)
            if result is not None:
                self.d[node_type]['PHR'].append(int(result.group(1)))
                self.d[node_type]['bler'].append(float(result.group(2)))
                self.d[node_type]['mcsoff'].append(int(result.group(3)))
                self.d[node_type]['mcs'].append(int(result.group(4)))


    def collect(self,testcase_id,node_type):
        #append each file's contents to another file (prepended with CI-) for debug
        for f in self.d[node_type]['files']:
            if os.path.isfile(f):
                cmd = 'cat '+ f + ' >> CI-'+testcase_id+'-'+f
                subprocess.Popen(cmd,shell=True)  
        #join the files for further processing
        cmd='cat '
        for f in self.d[node_type]['files']:
            if os.path.isfile(f):
                cmd += f+' '
        process=subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE)
        output = process.stdout.readlines()
        if node_type=='enb':
            self.process_enb(node_type,output)
        else: #'gnb'
            self.process_gnb(node_type,output)


    def graph(self,testcase_id, node_type):
        for page in self.d[node_type]['graph']:#work out a set a graphs per page
            col = 1
            figure, axis = plt.subplots(len(self.d[node_type]['graph'][page]), col ,figsize=(10, 10), sharex=True)
            i=0
            for m in self.d[node_type]['graph'][page]:#metric may refer to 1 level or 2 levels 
                metric_path=m.split('.')
                metric_l1=metric_path[0]
                if metric_path[0]=='ue':
                   metric_l2=metric_path[1]
                   major_ticks = np.arange(0, 10, 1)
                   for rnti in self.d[node_type]['rntis']:
                       tput = ''
                       if 'mbps' in metric_l2:
                          n = self.d[node_type]['test_params']['duration']
                          top_n = (sorted(self.d[node_type]['rntis'][rnti][metric_l2])[-n:])
                          tput = str(round(sum(top_n)/n,1))
                          tput = ' '+tput+' Mbps'
                       axis[i].plot(self.d[node_type]['rntis'][rnti][metric_l2],label=rnti+tput)
                       major_ticks = np.arange(0, len(self.d[node_type]['rntis'][rnti][metric_l2])+1, 1)
                       axis[i].set_xticks(major_ticks)
                   axis[i].set_xticklabels([])
                   axis[i].grid(axis='y')
                   axis[i].legend(loc='center left')
                   axis[i].set_xlabel('time (s)')
                   axis[i].set_ylabel(metric_l2)

                else:
                   if len(metric_path)==1:#1 level
                       major_ticks = np.arange(0, len(self.d[node_type][metric_l1])+1, 1)
                       axis[i].set_xticks(major_ticks)
                       axis[i].set_xticklabels([])
                       axis[i].plot(self.d[node_type][metric_l1],marker='o')
                       axis[i].set_xlabel('time')
                       axis[i].set_ylabel(metric_l1)
                       axis[i].set_title(metric_l1)

                   else:#2 levels
                       metric_l2=metric_path[1]
                       major_ticks = np.arange(0, len(self.d[node_type][metric_l1][metric_l2])+1, 1)
                       axis[i].set_xticks(major_ticks)
                       axis[i].set_xticklabels([])
                       axis[i].plot(self.d[node_type][metric_l1][metric_l2],marker='o')
                       axis[i].set_xlabel('time')
                       axis[i].set_ylabel(metric_l2)
                       axis[i].set_title(metric_l2)
                i+=1                

            plt.tight_layout()
            #save as png
            #plt.show()
            plt.savefig(node_type+'_stats_monitor_'+testcase_id+'_'+page+'.png')


if __name__ == "__main__":

    cfg_filename = sys.argv[1] #yaml file as metrics config
    testcase_id = sys.argv[2] #test case id to name files accordingly, especially if we have several tests in a sequence
    node = sys.argv[3]#enb or gnb
    mon=StatMonitor(cfg_filename)

    #collecting stats when modem process is stopped
    CMD='ps aux | grep modem | grep -v grep'
    process=subprocess.Popen(CMD, shell=True, stdout=subprocess.PIPE)
    output = process.stdout.readlines()
    while True:
        mon.collect(testcase_id,node)
        process=subprocess.Popen(CMD, shell=True, stdout=subprocess.PIPE)
        output = process.stdout.readlines()
        time.sleep(1)
        if len(output)==0 :
           break;
    print('Process stopped')
    with open(node+'_stats_monitor.pickle', 'wb') as handle:
        pickle.dump(mon.d, handle, protocol=pickle.HIGHEST_PROTOCOL)
    mon.graph(testcase_id, node)
