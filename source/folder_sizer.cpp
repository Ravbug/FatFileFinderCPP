//
//  folder_sizer.cpp
//  mac
//
//  Copyright © 2019 Ravbug. All rights reserved.
//

#include "folder_sizer.hpp"
#include "globals.cpp"
#include <array>

//constructor and destructor
folderSizer::folderSizer(){}
folderSizer::~folderSizer(){}

/**
 Calculate the size of a folder, including the size of subfolders
 @param folder the path to the folder to size
 @param progress the std::function to call with progress updates
 @return the FolderData structure representing the entire directory tree
 */
FolderData* folderSizer::SizeFolder(const string& folder, const progCallback& progress){
	if (folder.size() > maxPathLength){
		return NULL;
	}
	
	FolderData* fd = new FolderData{};
	fd->Path = path(folder);
	
	// iterate through the items in the folder
	try{
		for(auto& p : directory_iterator(folder)){
			//is the item a folder? if so, defer sizing it
			if (is_directory(p)){
				FolderData* sub = new FolderData{};
				sub->Path = p;
				fd->subFolders.push_back(sub);
			}
			else{
				//size the file, add its details to the structure
				FileData* file = new FileData{p,file_size(p)};
				fd->files_size += file->size;
				fd->files.push_back(file);
			}
		}
	}
	catch(exception){
		//notify user?
		return NULL;
	}
	fd->total_size = fd->files_size;
	fd->num_items = fd->files.size();
	
	//recursively size the folders in the folder
	float num = 0;
	for (int i = 0; i < fd->subFolders.size(); i++){
		fd->subFolders[i] = SizeFolder(fd->subFolders[i]->Path.string(), nullptr);
		if (fd->subFolders[i] != NULL){
			fd->num_items += fd->subFolders[i]->num_items + 1;
			fd->total_size += fd->subFolders[i]->total_size;
			num++;
			//check for zero size
			if (fd->total_size == 0){
				fd->total_size = 1;
			}
			
			if (progress != nullptr){
				progress(num / fd->subFolders.size(),fd);
			}
		}
	}

	return fd;
}

/**
 Formats a raw file size to a string with a unit
 @param fileSize the size of the item in bytes
 @returns unitized string, example "12 KB"
 */
string folderSizer::sizeToString(const unsigned long& fileSize){
	string formatted = "";
	int size = 1000;		//MB = 100, MiB = 1024
	array<string,5> suffix { " bytes", " KB", " MB", " GB", " TB" };

	for (int i = 0; i < suffix.size(); i++)
	{
		double compare = pow(size, i);
		if (fileSize <= compare)
		{
			int minus = 0;
			if (i > 0)
			{
				minus = 1;
			}
			//round to 2 decimal places, then attach unit
			char buffer[10];
			sprintf(buffer,"%.2f",fileSize / pow(size,i-minus));
			formatted = string(buffer) + suffix[i - minus];
			break;
		}
	}

	return formatted;
}

/**
 Recalculate the size and the number of items in a FolderData struct.
 Does not make filesystem calls, instead uses only the data in the FolderData struct.
 Modifies the properties of the struct.
 @param data the FolderData to resize
 */
void folderSizer::recalculateStats(FolderData* data){
	if (data->subFolders.size() > 0){
		data->num_items = 0;
		data->total_size = 0;
		data->files_size = 0;
		
		for(FolderData* sub : data->subFolders){
			folderSizer::recalculateStats(sub);
			data->num_items += sub->num_items;
			data->total_size += sub->total_size;
			data->files_size += sub->files_size;
		}
	}
}

