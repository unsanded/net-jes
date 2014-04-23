#ifndef __DROPBOX_FILE_H
#define __DROPBOX_FILE_H
#include "client.h"
#include <containers/jsonobject.h>
namespace http
{

namespace apis
{

namespace dropbox
{
class FileStream
{
	friend class FileMetaData;
	protected:
	Client& client;
	bool exists;
	int revision;
	http::Request downloadRequest;
	http::Request uploadRequest;
	

	string root;
	string path;
public:
	FileStream(Client& client, string path, string root="dropbox"):client(client),
    path(path), 
	root(root),
    downloadRequest( DROPBOX_API_CONTENT + ("files/" + root) + "/" + path),
	uploadRequest(   DROPBOX_API_CONTENT + ("files/" + root) + "/" + path, PUT)
	{
		client.authorise(downloadRequest);
		client.authorise(uploadRequest);
	}
	~FileStream()
	{
	}
	
	istream& download(){
		downloadRequest.reset();
		downloadRequest.send();
		return *downloadRequest.makeStream();
	}
	bool upload(istream& source){
		uploadRequest.reset();
		uploadRequest.send();
		boost::iostreams::copy(source, *uploadRequest.makeStream());
	}
	ostream& getUploadStream(){
		uploadRequest.reset();
		uploadRequest.send();
		
	}
};

class FileMetaData{
	Client& client;
	string root;
	string path;
	
	void readJson(JsonTree& tree){
		tree >> "size" > size;
		tree >> "rev" > rev ;
		tree >> "thumb_exists" > thumbExists;
		tree >> "is_dir" > isDir ;
		tree >> "bytes" > bytes;
		tree>>"modified" > modified;
		tree >> "icon" > icon;
		tree >> "mime_type" > mimeType;
		tree >> "revision"  > revision;
		tree >> "size" > size;
	}
	public:
	
	string size; // human-readable version of "bytes"
	string rev; // maybe this can be a long unsigned;
	bool thumbExists;
	bool isDir;
	long bytes;
	string modified;
	string icon;
	string mimeType;
	long revision;
	
	
	
	FileMetaData(Client& client, string path, string root="dropbox"): client(client), root(root), path(path){
		http::Request metaRequest(DROPBOX_API_CONTENT + ("metadata/" + root) + path);
		client.authorise(metaRequest);
		metaRequest.send();
		client.lastError=metaRequest.responseCode;
		if(client.lastError!=200)
			client.lastErrorMessage=metaRequest.responseMessage;
		
	}
	
	FileMetaData(FileStream& file): client(file.client), root(file.root), path(file.path){
		std::stringstream ss;
		string meta = file.downloadRequest.responseHeaders["x-dropbox-metadata"];
		ss << meta;
		containers::json::JsonTree metaTree(ss);
		readJson(metaTree);
	}
	
};
	



}}}//http::apis::dropbox

#endif // __DROPBOX_FILE_H
