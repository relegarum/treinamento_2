#!/usr/bin/env python
import sys
import os
from multiprocessing import Process
import time

PORT  = "2196"
IP = "localhost"

file_urls = IP +':' + PORT + '/ahoy.iso'
#file_urls = [IP +':' + PORT + '/pdfs/evolucaoDosCodecs.pdf',
#            IP + ':' + PORT + '/imgs/lena.jpeg',
#            IP +':' + PORT + '/imgs/mandrill.png',
#            IP +':' + PORT + '/teste_300mb.iso',
#            IP +':' + PORT + '/teste']
process_list = []

def download_file(file_url, fileout_name, i):
  print('Download start ---> '+ str(i)+ ' - ' + fileout_name)
  os.system('wget -nc  ' + file_url + ' -O arquivos_download/' 
    + fileout_name + "_" + str(i) )
  print('#### Download end ---> '+ str(i) +' - '+ fileout_name + " ")  
#os.system('curl -s --url 10.4.3.1:8080/teste_4gb.iso -o teste_4gb_ini.iso')
for i in range(0,2):
  print("-----------------------------------------------------")
  p = Process(target=download_file,
args=(file_urls,file_urls[file_urls.rfind('/')+1:],i, ))
  p.start()
  print("   #################  %d  ####################" % i)
#for i in xrange(len(file_urls)):
print("*******************************************************")
#  os.system("md5sum root/" + file_urls[i][file_urls[i].find('/')+1:]
 #   + " arquivos_download/" + file_urls[i][file_urls[i].rfind('/')+1:])
print("*******************************************************")

