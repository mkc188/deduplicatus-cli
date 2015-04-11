//
//  Dropbox.cpp
//  deduplicatus-cli
//
//  Created by Cheuk Hin Lam on 6/4/15.
//  Copyright (c) 2015 Peter Lam. All rights reserved.
//

//#include <iostream>

#include <sstream>
#include <string>
#include <curl/curl.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "WebAuth.h"
#include "Dropbox.h"
#include "tool.h"


#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace std;
using namespace rapidjson;
ostringstream stream_dropbox;

Dropbox::Dropbox(string token) {
    accessToken = token;

    // define cloud storage endpoints
    path_base = "https://api.dropbox.com/1";
    path_account_info = "/account/info";
}

string Dropbox::brandName() {
    return "Dropbox";
}

void Dropbox::accountInfo(Level *db, WebAuth *wa, string cloudid) {
    long curl_code = 0, http_code = 0;
    bool refreshOAuth = false, success = false;
    
    do {
        // init curl request
        CURL *curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, (path_base + path_account_info).c_str());
    
        // set oauth header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream_dropbox);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    
        curl_code = curl_easy_perform(curl);
        curl_slist_free_all(headers);
    
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        
        if( http_code == 200 && curl_code != CURLE_ABORTED_BY_CALLBACK ) {
            success = true;
            
            // parse response JSON for information
            Document d;
            d.Parse(stream_dropbox.str().c_str());
            
            Value& v_name = d["display_name"];
            Value& v_quota = d["quota_info"]["quota"];
            Value& v_normal = d["quota_info"]["normal"];
            Value& v_shared = d["quota_info"]["shared"];
            
            displayName = v_name.GetString();
            space_quota = v_quota.GetUint64();
            space_used = v_normal.GetUint64() + v_shared.GetUint64();
            
        } else {
            // dropbox api do not implement refresh token mechanism
            refreshOAuth = true;
        }
    } while( !success && !refreshOAuth );
}

void Dropbox::uploadFile(string local, string remoteFolderName) {
//    cout << local << endl;
    
//    file_stream<unsigned char>::open_istream(local)
//    .then([](pplx::task<concurrency::streams::basic_istream<unsigned char>> previousTask)
//          {
//              try
//              {
//                  auto fileStream = previousTask.get();
//                  
//                  // Make HTTP request with the file stream as the body.
//                  http_client client("http://www.fourthcoffee.com");
//                  return client.request(methods::PUT, "myfile", fileStream)
//                  .then([fileStream](pplx::task<http_response> previousTask)
//                        {
//                            fileStream.close();
//                            
//                            std::wostringstream ss;
//                            try
//                            {
//                                auto response = previousTask.get();
//                                ss << L"Server returned returned status code " << response.status_code() << L"." << std::endl;
//                            }
//                            catch (const http_exception& e)
//                            {
//                                ss << e.what() << std::endl;
//                            }
//                            std::wcout << ss.str();
//                        });
//              }
//              catch (const std::system_error& e)
//              {
//                  std::wostringstream ss;
//                  ss << e.what() << std::endl;
//                  std::wcout << ss.str();
//                  
//                  // Return an empty task. 
//                  return pplx::task_from_result();
//              }
//          }).wait(); 
    
    //Build the proxy
    http_client_config client_config;
    
    web_proxy wp(web_proxy::use_auto_discovery);
    client_config.set_proxy(wp);
 
    // Open a stream to the file to write the HTTP response body into.
    auto fileBuffer = std::make_shared<streambuf<uint8_t>>();
    file_buffer<uint8_t>::open("haha", std::ios::out)
    .then([=](streambuf<uint8_t> outFile) -> pplx::task<http_response>
          {
              *fileBuffer = outFile; 
              
              // Create an HTTP request.
              // Encode the URI query since it could contain special characters like spaces.
              http_client client(U("http://www.bing.com/"), client_config);
              return client.request(methods::GET, uri_builder(U("/search")).append_query(U("q"), "fuck").to_string());
          })
    
    // Write the response body into the file buffer.
    .then([=](http_response response) -> pplx::task<size_t>
          {
              printf("Response status code %u returned.\n", response.status_code());
              
              return response.body().read_to_end(*fileBuffer);
          })
    
    // Close the file buffer.
    .then([=](size_t)
          {
              return fileBuffer->close();
          })
    
    // Wait for the entire response body to be written into the file.
    .wait();
}