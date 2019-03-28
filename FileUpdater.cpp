#include "Network.cpp"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/stat.h>
#define DIRECTORY_PATH "/home/sameeksha/Desktop/Data"

class FileUpdater:public ServerSocket,public ClientSocket
{
public:
	static FILE *fpToUpdate;
	static char *fileContentToUpdate;
	struct FileInfo
	{
		char *fileNameInfo;
		time_t modificationTimeInfo;
	};	

	FileUpdater()
	{

	}	
	FileUpdater(char *IPAddress,int PortNumber)
	{	
		AddHost(IPAddress,PortNumber);
	}
	virtual void SendUpdate(char *directoryPath,int PortNumber)
	{
		DIR *dirp=opendir(directoryPath);
		struct stat statFilePointer;		
		struct dirent *fileEntry;		
		struct dirent *entry;

	if(dirp == NULL)
	{
		printf("Specified File Directory does not exists\n");
	}
	else
	{
	 int numFileCounter=0;	 
 
	 while((entry=readdir(dirp)) != NULL)
	 {
		numFileCounter=numFileCounter + 1;
	 }
	 
	 closedir(dirp);

	 FileInfo objFileInfo[numFileCounter];

	 for(int i=0;i<numFileCounter;i++)
	 {
		objFileInfo[i].fileNameInfo=NULL;
	 }
	 
	 dirp=opendir(directoryPath); 

	 while(PortNumber != 0)
	 {	     
	   while((fileEntry=readdir(dirp)) != NULL)
	   {				
		char *fullPath=(char *)malloc(sizeof(char)*((strlen(directoryPath) + strlen(fileEntry->d_name) + 4)));
		
		sprintf(fullPath,"%s/%s",directoryPath,fileEntry->d_name);
		if((strcmp(fileEntry->d_name,".") != 0) && (strcmp(fileEntry->d_name,"..")!=0))
		{
			char *tildaSuffix=strrchr(fullPath,'~');
			if(tildaSuffix == NULL)
			{
				FILE *fp=fopen(fullPath,"r");
				
				mode_t fileMode;
				int status=stat(fullPath,&statFilePointer);
				fileMode= statFilePointer.st_mode;
			
				if(S_ISREG(fileMode) != 0)
				{
					time_t modifiedTime;
					time_t currentTime;
					modifiedTime = statFilePointer.st_mtime;
					currentTime=time(NULL);

					int diffSecTime= difftime(currentTime,modifiedTime);
					if(diffSecTime <= 180)
					{
						int Transferfile=1;
						int FileExists=0;
						int numFileTransferred=-1;
						char *fileContent=NULL;
						off_t fSize=statFilePointer.st_size;
						int fileSize=(int)fSize;

						for(int i=0;i< numFileCounter;i++)
						{
							if(objFileInfo[i].fileNameInfo != NULL)
							{
									numFileTransferred=numFileTransferred + 1;
								if(strcmp(objFileInfo[i].fileNameInfo,fileEntry->d_name) == 0)
								{
									FileExists=1;

									time_t modificationTimeSent=objFileInfo[i].modificationTimeInfo;
									int diffModTime=difftime(modifiedTime,modificationTimeSent);
									if(diffModTime == 0)
									{
										Transferfile=0;
										break;
									}
									else if (diffModTime > 0)
									{
										Transferfile=1;
								objFileInfo[i].modificationTimeInfo=modifiedTime;
fileContent=NULL;
fileContent=(char *)realloc(fileContent,sizeof(char)*(fileSize + 4));
										break;	
									}
								}
								else
								{
									Transferfile=1;
									FileExists=0;
								}
							}
						}
						if(FileExists == 0)
						{
							numFileTransferred=numFileTransferred + 1;
							objFileInfo[numFileTransferred].fileNameInfo=fileEntry->d_name;
							objFileInfo[numFileTransferred].modificationTimeInfo=modifiedTime;
fileContent=NULL;
fileContent=(char *)malloc(sizeof(char)*(fileSize + 4));
						}
						if(Transferfile == 1)
						{
							char data[1024];
							char *fileName;
							
							fileName=fileEntry->d_name;
							SendData(1,(char *)"FileName",fileName);

							char *rcv=fgets(data,1024,fp);
							while(rcv != NULL)
							{
					
								strncat(fileContent,data,(strlen(data)));
								memset((void*)data, 0, 1024);
								rcv=fgets(data,1024,fp);
							}
							SendData(2,(char *)"FileSize",fileContent);
							SendData(3,(char *)"FileContent",fileContent);							
							fclose(fp);
						}
						else
						{
							fclose(fp);
						}		
					}
					else
					{	
						fclose(fp);
					}
				}
				else
				{
					fclose(fp);
				}
				
			}
		}
	}
	rewinddir(dirp);
      }
	closedir(dirp);
     }	
   }

	virtual void ProcessUpdate(char *dataRcv)
	{
		char *splitData;
		char *VariableName;
		char *VariableContent;
		int counter=0;

		splitData=strtok(dataRcv,"@");
		
		while(splitData != NULL)
		{
			counter=counter + 1;
			if(counter == 1) //Variable Name corresponds to counter=1
			{
				VariableName=splitData;
			}
			if(counter == 2)// Variable Content correposnds to counter=2
			{
				VariableContent=splitData;
				if(strcmp(VariableName,"FileName")==0)
				{
					char *fullPath=(char *)malloc(sizeof(char)*((strlen(serverDirectoryPath) + strlen(VariableContent) + 4)));
					
					sprintf(fullPath,"%s/%s",serverDirectoryPath,VariableContent);
printf("%s\n",fullPath);				
					fpToUpdate=fopen(fullPath,"w");
					
					free(fullPath);
					lendataReceived=20;
				}				
				if(strcmp(VariableName,"FileSize")==0)
				{
					fileContentToUpdate=NULL;
					fileContentToUpdate=(char *)realloc(fileContentToUpdate,sizeof(char)*(atoi(VariableContent)));
					lendataReceived=20;
				}
				if(strcmp(VariableName,"FileContent")==0)
				{
					fileContentToUpdate=splitData;
					printf("%s\n",fileContentToUpdate);
					fputs(fileContentToUpdate,fpToUpdate);
					
					lendataReceived=20;										
					fclose(fpToUpdate);
				}
				counter=0;
			}
			splitData=strtok(NULL,"@!");
		}
	}
};

FILE *FileUpdater::fpToUpdate=NULL;
char *FileUpdater::fileContentToUpdate=(char *)malloc(sizeof(char)*(20));

int main(int argc,char* argv[])
{
	int iPortNumber= 0;
	if(argv[3]) iPortNumber = atoi(argv[3]);
	if (argv[2])
	{		
		FileUpdater fUpdater(argv[2],iPortNumber);		
		while(iPortNumber != 0)
		{
			if(argv[1])
			{
				fUpdater.SendUpdate(argv[1],iPortNumber);
			}			
			fUpdater.SendUpdate((char *)DIRECTORY_PATH,iPortNumber);
		}
	}
	else 
	{	
		iPortNumber=8221;
		FileUpdater fUpdater;
		fUpdater.serverDirectoryPath=argv[1];
		while(iPortNumber != 0)
		{			
		
		}
	}
}
