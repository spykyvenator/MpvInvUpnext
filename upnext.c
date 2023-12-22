#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <mpv/client.h>

#define PREFIX "http://192.168.0.11:7665/watch?v="

// Callback function to capture HTTP response data
static size_t 
WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char* data = (char*)userp;
    strcat(data, (char*)contents);
    return realsize;
}

char *
getNextUrl(char *url) {

    CURL* curl;
    CURLcode res;
    char html[100000] = "";

    // Initialize libcurl
    curl = curl_easy_init();
    if (curl) {
        // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Configure libcurl to capture the HTTP response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);

        // Perform the HTTP request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed\n"); //;, curl_easy_strerror(res));
        } else {
            // Find the line containing 'tabindex="-1" href="/watch?v='
            char* line = strstr(html, "tabindex=\"-1\" href=\"/watch?v=");
            if (line) {
                // Extract the video ID
                line += strlen("tabindex=\"-1\" href=\"/watch?v=");
                char* video_id_end = strchr(line, '"');
                if (video_id_end)
                    *(video_id_end+1) = '\0';
                video_id_end = strchr(line, '<');
                if (video_id_end)
                    *(video_id_end+1) = '\0';
                video_id_end = strchr(line, '&');
                if (video_id_end)
                    *(video_id_end+1) = '\0';
            }
            size_t i = strlen(PREFIX)+strlen(line);
            char *buffer = malloc(sizeof(char)*i);
            snprintf(buffer, i, PREFIX"%s", line);
            return buffer;
        }

        // Clean up libcurl
        curl_easy_cleanup(curl);
    }
    return NULL;

}

int
mpv_open_cplugin(mpv_handle *mpv)
{
    while (1) {
        mpv_event *event = mpv_wait_event(mpv, -1);
        if (event->event_id == MPV_EVENT_START_FILE){
            char *url = mpv_get_property_string(mpv, "path");
            char *next = getNextUrl(url);
            const char *cmd[] = { "loadfile", next, "append", NULL };
            printf("\nurl: %s\ncmd: %s\n", url, cmd[1]);
            mpv_command_async(mpv, 0, cmd);
            mpv_free(url);
            free(next);
        }
        if (event->event_id == MPV_EVENT_SHUTDOWN)
            break;
    }
    return 0;
}
