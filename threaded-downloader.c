#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <regex.h>
#include <string.h>

#include <curl/curl.h>


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
void *download_thread(char queue[300][150], int threads);
void *curl_download(void * url);


int main(void) {
  curl_global_init(CURL_GLOBAL_ALL);

  printf("USAGE: Enter urls into the prompt, one at a time.\n");
  printf("When you have entered all the urls you wish to download, enter download to start.\n");
  printf("Files are saved in the present working directory\n");
  printf("Press enter to start");
  getchar();
  int i;
  char command[150];
  char queue[300][150];
  for(i = 0; i < 300; ++i) {
    printf("URL: ");
    scanf("%s", command);
    if((strncmp(command, "download", 150)) == 0){
      download_thread(queue, i);
      break;
    }
    else {
        sscanf(command, "%s", queue[i]);
    }
  }

  return 0;
}


static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
  return written;
}

void *download_thread(char queue[300][150], int threads) {
  int i;
  pthread_t thread[threads];
  for(i = 0; i < threads; i++){
    int ret = pthread_create(&thread[i], NULL, curl_download, &queue[i]);
    if(ret != 0) {
      printf("PTHREAD ERROR");
      exit(1);
    }
  }
  for(i = 0; i< threads; i++)
    pthread_join(thread[i], NULL);
}

void *curl_download(void * url){
  CURL *downloader;
  regex_t regex;
  regmatch_t m[1];
  char filepath[150];
  int match = regcomp(&regex, "[^\\/]*$", 0);
  match = regexec(&regex,(char *) url, 1, m, 0);
  if(!match){
    int start = m[0].rm_so;
    int end = m[0].rm_eo; 
    strncpy(filepath, (char *) url + start, end);
  }
  else
    sscanf(filepath, "%s",(char *) url);
  char *pagefilename = filepath;
  FILE *pagefile;

  downloader = curl_easy_init();
  curl_easy_setopt(downloader, CURLOPT_URL, url);
  curl_easy_setopt(downloader, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(downloader, CURLOPT_WRITEFUNCTION, write_data);
  pagefile = fopen(pagefilename, "wb");
  
  if (pagefile) {
    curl_easy_setopt(downloader, CURLOPT_FILE, pagefile);
    curl_easy_perform(downloader);
    fclose(pagefile);
  }
  curl_easy_cleanup(downloader);
  printf("%s DONE\n",(char *)url);
}