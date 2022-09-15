/*Copyright 2008-2022 - Loïc Le Cunff

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/

#include <gui_rsc.h>

std::vector<std::string> ImagesManager::paths;
std::vector<wxImage> ImagesManager::images;
std::vector<wxBitmap> ImagesManager::bitmaps;

int ImagesManager::get_index(std::string path)
{
    unsigned int i,N=paths.size();
    
    int out=0;
    
    bool unknown=true;
    
    for(i=0;i<N;i++)
    {
        if(path==paths[i])
        {
            unknown=false;
            out=i;
            break;
        }
    }
    
    if(unknown)
    {
        wxImage tmp_image;
        tmp_image.AddHandler(new wxPNGHandler);
        tmp_image.LoadFile(path,wxBITMAP_TYPE_PNG);
        
        wxBitmap tmp_bitmap(tmp_image);
        
        paths.push_back(path);
        images.push_back(tmp_image);
        bitmaps.push_back(tmp_bitmap);
        
        out=N;
    }
    
    return out;
}

wxBitmap ImagesManager::get_bitmap(std::string path){ return bitmaps[get_index(path)]; }
wxImage ImagesManager::get_image(std::string path){ return images[get_index(path)]; }
