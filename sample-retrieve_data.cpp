
//NOTE: Query does not filter the incoming data yet

//Code is built upon sample code written by TroubleChute from his curl with Visual tudio tutorial
//inks to  the sample code used: https://tcno.co/TroubleChute/info/curl-vs2019/
//link to the youtue video where I discovered the code: https://youtu.be/q_mXVZ6VJs4?si=jNFIei1Xhy1W0odV

#define CURL_STATICLIB
#include <iostream> //For Debug
#include <fstream> //Write output json file
#include <string> //Process Data as a string (Could be char *, but string is safer)
#include <iomanip> //Algorithm For setw() function in json file to make it look better
#include "curl.h" //to faciliate HTTP request to overpass API //Linked statically


//function below taken directly from TC Sample code, function is called to write data to a string
static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
    std::string& text = *static_cast<std::string*>(param);
    size_t totalsize = size * nmemb;
    text.append(static_cast<char*>(buffer), totalsize);
    return totalsize;
}

int main()
{   
    std::string result;
    CURL* curl; 
    CURLcode return_code;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl == NULL)
    {
        std::cout << "HTTP request failed" << std::endl;
        return -1;
    }

    //where area_to_survey is formatted as "(north west corner lat, north west corner long, south east corner lat, south east corner long)
    const std::string area_to_survey = "(53.052365 , -2.537772 , 53.075159 , -2.500410 )";
    const std::string request = "data=[out:json];way[highway]" + area_to_survey + ";out geom; ";

    const char* request_payload = request.c_str(); //make a variable part of a function
    

    curl_easy_setopt(curl,  CURLOPT_URL, "https://overpass-api.de/api/interpreter");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

    //uncommend below line for extra information about http request
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    return_code = curl_easy_perform(curl);

    if (return_code != CURLE_OK)
    {
        std::cout << "Error:" << std::endl;
        std::cout << result << std::endl;
        std::cout << curl_easy_strerror(return_code);
        return -1;
    }

    //close session
    curl_easy_cleanup(curl); 

    std::ofstream output("Output/Single_Chunk_JSON.json", std::ofstream::out);
    output << std::setw(4) << result << std::endl;
    output.close();

    std::cout << "API SESSION CLOSED\n";
    return 0;
}