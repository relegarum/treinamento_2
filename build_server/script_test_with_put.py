#!/usr/bin/env python
import sys
import os
from multiprocessing import Process
import time

PORT  = "2196"
IP = "10.4.11.2"
put_file = 'suse_default.iso'
file_urls = 'http://' + IP +':' + PORT + '/' + put_file
#file_urls = [IP +':' + PORT + '/pdfs/evolucaoDosCodecs.pdf',
#            IP + ':' + PORT + '/imgs/lena.jpeg',
#            IP +':' + PORT + '/imgs/mandrill.png',
#            IP +':' + PORT + '/teste_300mb.iso',
#            IP +':' + PORT + '/teste']

def upload_file(index):
  print("##### PUT TEST ####")
  os.system("curl http://"+ IP +":" + PORT + "/arquivos_download/" + put_file + ".put_" + str(index) + " --upload-file " + put_file + " --verbose --progress-bar");

def download_file(file_url, fileout_name, i):
  print('Download start ---> '+ str(i)+ ' - ' + fileout_name)
  os.system('wget -nc  ' + file_url + ' -O arquivos_download/'
    + fileout_name + "_" + str(i) )
  print('#### Download end ---> '+ str(i) +' - '+ fileout_name + " ")


os.system('rm arquivos_download/*')

for i in range(0,4):
  print("-----------------------------------------------------")
  if (i % 2 == 0):
    p = Process(target=download_file,args=(file_urls,file_urls[file_urls.rfind('/')+1:],i, ))
    p.start()
  else:
    p = Process(target=upload_file, args=(i, )).start()
  print("   #################  %d  ####################" % i)
#for i in xrange(len(file_urls)):
print("*******************************************************")
#  os.system("md5sum root/" + file_urls[i][file_urls[i].find('/')+1:]
 #   + " arquivos_download/" + file_urls[i][file_urls[i].rfind('/')+1:])
print("*******************************************************")

