#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/uio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <libgen.h>

#include "protocol.h"
#include "validate.h"
#include "status.h"
#include "stack.h"
char current_user[255];
char root[100];
int requestId;
int client_sock;
struct sockaddr_in server_addr;
char choose;
Message *mess;
int isOnline = 0;
struct Stack *stack;
char *listCurrentDirec[30];
char **listFolder;
char **listFile;
#define DIM(x) (sizeof(x) / sizeof(*(x)))

void openFolder(char *folder);

int numberElementsInArray(char **temp)
{
	int i;
	if (temp != NULL)
	{
		for (i = 0; *(temp + i); i++)
		{
			// count number elements in array
		}
		return i;
	}
	return 0;
}

int hasInList(char *element, char **temp)
{
	int i;
	if (temp != NULL)
	{
		for (i = 0; *(temp + i); i++)
		{
			if (strcmp(element, temp[i]) == 0)
				return 1;
		}
	}
	return 0;
}

int initSock()
{
	int newsock = socket(AF_INET, SOCK_STREAM, 0);
	if (newsock == -1)
	{
		perror("\nError: ");
		exit(0);
	}
	return newsock;
}

void *showBubbleNotify(char *notify)
{
	char command[200];
	sprintf(command, "terminal-notifier -message \"%s\"", notify);
	system(command);
	return NULL;
}

void bindClient(int port, char *serverAddr)
{
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(serverAddr);
}

void toNameOfFile(char *fileName, char *name)
{
	char **tokens = str_split(fileName, '/');
	int i;
	for (i = 0; *(tokens + i); i++)
	{
		// count number elements in array
	}
	strcpy(name, *(tokens + i - 1));
}
void removeFile(char *fileName)
{
	// remove file
	if (remove(fileName) != 0)
	{
		perror("Following error occurred\n");
	}
}

void getDirectory()
{
	Message sendMsg, recvMsg1, recvMsg2;
	sendMsg.type = TYPE_REQUEST_DIRECTORY;
	sendMsg.requestId = requestId;
	sendMsg.length = 0;
	sendMessage(client_sock, sendMsg);
	// receive list path, list folder path, list file path from server
	receiveMessage(client_sock, &recvMsg1);
	receiveMessage(client_sock, &recvMsg2);
	if (recvMsg1.length > 0)
		listFolder = str_split(recvMsg1.payload, '\n');
	if (recvMsg2.length > 0)
		listFile = str_split(recvMsg2.payload, '\n');
}

void showDirectory(char *root)
{
	printf("\n---------------- Your Directory ----------------\n");
	int i;
	int j = 0;
	printf("   %-15s%-30s%-6s\n", "Name", "Path", "Type");
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			char *temp = strdup(listFolder[i]);
			char *temp2 = strdup(listFolder[i]);
			if (strcmp(dirname(temp), root) == 0)
			{
				printf("%d. %-15s%-30sFolder\n", j + 1, basename(temp2), listFolder[i]);
				listCurrentDirec[j] = strdup(listFolder[i]);
				j++;
			}
			free(temp);
			free(temp2);
		}
	}
	if (numberElementsInArray(listFile) > 0)
	{
		for (i = 0; *(listFile + i); i++)
		{
			char *temp = strdup(listFile[i]);
			char *temp2 = strdup(listFile[i]);
			if (strcmp(dirname(temp), root) == 0)
			{
				printf("%d. %-15s%-30sFile\n", j + 1, basename(temp2), listFile[i]);
				listCurrentDirec[j] = strdup(listFile[i]);
				j++;
			}
			free(temp);
			free(temp2);
		}
	}
}

void uploadMultiFile()
{
	char fullPath[100];
	int i = 0;
	printf("\n------------------ Upload File ------------------\n");
	printf("Please choose folder you want to upload into it :\n");
	printf("1. %-15s./%-28sFolder\n", current_user, current_user);
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			char *temp = strdup(listFolder[i]);
			printf("%d. %-15s%-30sFolder\n", i + 2, basename(temp), listFolder[i]);
			free(temp);
		}
	}
	char choose[10];
	int option;
	while (1)
	{
		printf("\nChoose (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i + 1))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
		return;
	printf("Please input the path of file you want to upload:");
	scanf("%[^\n]s", fullPath);

	char **temp = str_split(&fullPath[0], ' ');

	Message msg, sendMsg, recvMsg;

	int num_file = atoi(temp[0]);
	char message[20] = "";
	for (int i = 0; i < atoi(temp[0]); i++)
	{
		FILE *fptr;
		char fileName[30] = "";
		if ((fptr = fopen(temp[i + 1], "rb+")) == NULL)
		{
			printf("Error: File %s not found\n", temp[i + 1]);
			num_file -= 1;
		}
		else
		{
			toNameOfFile(temp[i + 1], fileName);
			if (option == 1)
			{
				char file[1024];
				sprintf(&file[0], "_./%s/%s", current_user, fileName);
				strcat(message, file);
			}
			else
			{
				char file[1024];
				// strcat(file, fileName);
				sprintf(file, "_./%s/%s", listFolder[option - 2], fileName);
				strcat(message, file);
			}
		}
		fclose(fptr);
	}

	char c = num_file + '0';
	sprintf(msg.payload, "%c%s", c, message);
	printf("%s\n", msg.payload);
	msg.type = TYPE_UPLOAD_FILE;
	msg.length = strlen(msg.payload);
	msg.requestId = requestId;
	sendMessage(client_sock, msg);
	receiveMessage(client_sock, &recvMsg);

	if (recvMsg.type == TYPE_ERROR)
	{
		printf("%s\n", recvMsg.payload);
	}
	else
		for (int i = 0; i < atoi(temp[0]); i++)
		{
			FILE *fptr;
			char fileName[30] = "";
			if ((fptr = fopen(temp[i + 1], "rb+")) != NULL)
			{
				long filelen;
				fseek(fptr, 0, SEEK_END); // Jump to the end of the file
				filelen = ftell(fptr);	  // Get the current byte offset in the file
				rewind(fptr);			  // pointer to start of file
				int sumByte = 0;
				while (!feof(fptr))
				{
					int numberByteSend = PAYLOAD_SIZE;
					if ((sumByte + PAYLOAD_SIZE) > filelen)
					{ // if over file size
						numberByteSend = filelen - sumByte;
					}
					char *buffer = (char *)malloc((numberByteSend) * sizeof(char));
					fread(buffer, numberByteSend, 1, fptr); // read buffer with size
					memcpy(sendMsg.payload, buffer, numberByteSend);
					sendMsg.length = numberByteSend;
					sumByte += numberByteSend; // increase byte send
					if (sendMessage(client_sock, sendMsg) <= 0)
					{
						printf("Connection closed!\n");
						break;
					}
					free(buffer);
					if (sumByte >= filelen)
					{
						break;
					}
				}
				sendMsg.length = 0;
				sendMessage(client_sock, sendMsg);
				receiveMessage(client_sock, &recvMsg);
				printf("%s\n", recvMsg.payload);
			}
			fclose(fptr);
		}
}

void handleSearchFile(char *fileName, char *listResult)
{
	int i;
	for (i = 0; *(listFile + i); i++)
	{
		char *temp = strdup(listFile[i]);
		if (strcmp(basename(temp), fileName) == 0)
		{
			strcat(listResult, listFile[i]);
			strcat(listResult, "\n");
		}
		free(temp);
	}
}

int handleSelectDownloadFile(char *selectLink)
{
	char fileName[100];
	char listResult[1000];
	memset(listResult, '\0', sizeof(listResult));
	printf("\n------------------ Download File ------------------\n");
	printf("Please Input Download File Name: ");
	scanf("%[^\n]s", fileName);
	handleSearchFile(fileName, listResult);
	if (strlen(listResult) <= 0)
		return -1;
	char **tmp = str_split(listResult, '\n');
	int i;
	printf("   %-15s%-30s\n", "Name", "Path");
	for (i = 0; *(tmp + i); i++)
	{
		printf("%d. %-15s%-30s\n", i + 1, fileName, tmp[i]);
	}
	char choose[10];
	int option;
	while (1)
	{
		printf("\nPlease select to download (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}

	if (option == 0)
	{
		return -1;
	}
	else
	{
		strcpy(selectLink, tmp[option - 1]);
	}
	return 1;
}

int download(char *link)
{
	Message sendMsg, recvMsg;
	FILE *fptr;
	char saveFolder[20];
	char savePath[50];
	char temp[50];
	strcpy(temp, link);
	sendMsg.type = TYPE_REQUEST_DOWNLOAD;
	sendMsg.requestId = requestId;
	strcpy(sendMsg.payload, link);
	sendMsg.length = strlen(sendMsg.payload);
	sendMessage(client_sock, sendMsg);
	receiveMessage(client_sock, &recvMsg);
	if (recvMsg.type != TYPE_ERROR)
	{
		printf("Please Input Saved Path in Local: ");
		scanf("%[^\n]s", saveFolder);
		sprintf(savePath, "%s/%s", saveFolder, basename(temp));
		if (fopen(savePath, "r+") != NULL)
		{
			char choose;
			printf("Warning: File name already exists!!! Do you want to replace? Y/N\n");
			while (1)
			{
				scanf(" %c", &choose);
				while (getchar() != '\n')
					;
				if ((choose == 'Y') || choose == 'y' || choose == 'N' || choose == 'n')
				{
					break;
				}
				else
				{
					printf("Please press Y or N\n");
				}
			}
			if (choose == 'N' || choose == 'n')
			{
				sendMsg.type = TYPE_CANCEL;
				sendMessage(client_sock, sendMsg);
				return -1;
			}
		}
		sendMsg.type = TYPE_OK;
		sendMessage(client_sock, sendMsg);
		printf("----------------------Downloading-----------------------\n");
		fptr = fopen(savePath, "w+");
		while (1)
		{
			receiveMessage(client_sock, &recvMsg);
			if (recvMsg.type == TYPE_ERROR)
			{
				fclose(fptr);
				removeFile(savePath);
				return -1;
			}
			if (recvMsg.length > 0)
			{
				fwrite(recvMsg.payload, recvMsg.length, 1, fptr);
			}
			else
			{
				break;
			}
		}
		fclose(fptr);
		return 1;
	}
	return -1;
}

void downloadFile()
{
	char selectLink[50];
	if (handleSelectDownloadFile(selectLink) == 1)
	{
		printf("...............................................\n");
		if (download(selectLink) == -1)
		{
			printf("Having trouble, stop downloading the file!!\n");
			return;
		}
		printf("Donwload Successful!!!");
	}
	else
	{
		printf("No results were found\n");
		return;
	}
}
void open(char *pre_folder, char *cur_folder)
{
	push(stack, pre_folder);
	openFolder(cur_folder);
}

void createNewFolder()
{
	int i = 0;
	printf("\n------------------ Create Folder ------------------\n");
	printf("Please choose folder you want to create into it :\n");
	printf("1. %-15s./%-28sFolder\n", current_user, current_user);
	if (numberElementsInArray(listFolder) > 0)
	{
		for (i = 0; *(listFolder + i); i++)
		{
			char *temp = strdup(listFolder[i]);
			printf("%d. %-15s%-30sFolder\n", i + 2, basename(temp), listFolder[i]);
			free(temp);
		}
	}
	char choose[10];
	int option;
	while (1)
	{
		printf("\nChoose (Press 0 to cancel): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i + 1))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
		return;
	char newFolder[20];
	Message sendMsg;
	printf("Input new folder name: ");
	scanf("%[^\n]s", newFolder);
	if (option == 1)
	{
		strcpy(sendMsg.payload, root);
	}
	else
	{
		strcpy(sendMsg.payload, listFolder[option - 2]);
	}
	strcat(sendMsg.payload, "/");
	strcat(sendMsg.payload, newFolder);
	printf("%s", sendMsg.payload);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_CREATE_FOLDER;
	sendMessage(client_sock, sendMsg);
}
void deleteFile(char *cur_file)
{
	Message sendMsg;
	strcpy(sendMsg.payload, cur_file);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_DELETE_FILE;
	sendMessage(client_sock, sendMsg);
}

void menuFileProcess()
{
	printf("\n------------------File Process------------------\n");
	printf("\n1 - Delete File");
	printf("\n2 - Download File");
	printf("\n3 - Cancel");
	printf("\nChoose: ");
}
void fileProcess(char *cur_file)
{
	menuFileProcess();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		deleteFile(cur_file);
		getDirectory();
		break;
	case '2':

		break;
	case '3':
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

void deleteFolder(char *cur_folder)
{
	Message sendMsg;
	strcpy(sendMsg.payload, cur_folder);
	sendMsg.length = strlen(sendMsg.payload);
	sendMsg.requestId = requestId;
	sendMsg.type = TYPE_DELETE_FOLDER;
	sendMessage(client_sock, sendMsg);
}

void menuFolderProcess()
{
	printf("\n------------------Folder Process------------------\n");
	printf("\n1 - Open");
	printf("\n2 - Delete Folder");
	printf("\n3 - Cancel");
	printf("\nChoose: ");
}
void folderProcess(char *pre_folder, char *cur_folder)
{
	menuFolderProcess();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		open(pre_folder, cur_folder);
		break;
	case '2':
		deleteFolder(cur_folder);
		getDirectory();
		// openFolder(cur_folder);
		break;
	case '3':
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

void openFolder(char *folder)
{
	showDirectory(folder);
	// for(int u=0;u<=stack->top;u++)
	// printf("++++++Top Stack++++++: %s\n",stack->array[u]);
	printf("\n------------------------------------------------\n");
	char choose[10];
	int option;
	int i = numberElementsInArray(listCurrentDirec);
	while (1)
	{
		printf("\nSelect to File/Folder (1,2...n)(Press 0 to Back): ");
		scanf(" %s", choose);
		while (getchar() != '\n')
			;
		option = atoi(choose);
		if ((option >= 0) && (option <= i))
		{
			break;
		}
		else
		{
			printf("Please Select Valid Options!!\n");
		}
	}
	if (option == 0)
	{
		if (!isEmpty(stack))
			openFolder(pop(stack));
		else
			return;
	}
	else
	{
		if (hasInList(listCurrentDirec[option - 1], listFolder))
		{
			folderProcess(folder, listCurrentDirec[option - 1]);
		}
		else
		{
			fileProcess(listCurrentDirec[option - 1]);
		}
	}
}

// connect client to server
// parameter: client socket, server address
// if have error, print error and exit
void connectToServer()
{
	if (connect(client_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately!\n");
		exit(0);
	}
}

void printWatingMsg()
{
	printf("\n..................Please waiting................\n");
}

// get username and password from keyboard to login
void getLoginInfo(char *str)
{
	char username[255];
	char password[255];
	printf("Enter username: ");
	scanf("%[^\n]s", username);
	while (getchar() != '\n')
		;
	printf("Enter password: ");
	scanf("%[^\n]s", password);
	while (getchar() != '\n')
		;
	sprintf(mess->payload, "USER %s\nPASS %s", username, password);
	strcpy(str, username);
}

void loginFunc(char *current_user)
{
	char username[255];
	mess->type = TYPE_LOGIN;
	getLoginInfo(username);
	mess->length = strlen(mess->payload);

	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);

	if (mess->type != TYPE_ERROR)
	{
		isOnline = 1;
		strcpy(current_user, username);
		strcpy(root, "./");
		strcat(root, username);
		requestId = mess->requestId;
		getDirectory();
		stack = createStack(30);
		printf("%s\n", mess->payload);
	}
	else
	{
		printf("%s\n", mess->payload);
	}
}

int getRegisterInfo(char *user)
{
	char username[255], password[255], confirmPass[255];
	printf("Username: ");
	scanf("%[^\n]s", username);
	printf("Password: ");
	while (getchar() != '\n')
		;
	scanf("%[^\n]s", password);
	printf("Confirm password: ");
	while (getchar() != '\n')
		;
	scanf("%[^\n]s", confirmPass);
	while (getchar() != '\n')
		;
	if (!strcmp(password, confirmPass))
	{
		sprintf(mess->payload, "USER %s\nPASS %s", username, password);
		strcpy(user, username);
		return 1;
	}
	else
	{
		printf("Confirm password invalid!\n");
		return 0;
	}
}

void registerFunc(char *current_user)
{
	char username[255];
	if (getRegisterInfo(username))
	{
		mess->type = TYPE_REGISTER;
		mess->length = strlen(mess->payload);
		sendMessage(client_sock, *mess);
		receiveMessage(client_sock, mess);
		if (mess->type != TYPE_ERROR)
		{
			isOnline = 1;
			strcpy(current_user, username);
			strcpy(root, "./");
			strcat(root, username);
			requestId = mess->requestId;
			getDirectory();
			stack = createStack(30);
		}
		else
		{
			showBubbleNotify("Error: Register Failed!!");
		}
		// printf("%s\n", mess->payload);
		printf("REGISTER SUCCESSFUL!!!\n");
	}
}

void logoutFunc(char *current_user)
{
	mess->type = TYPE_LOGOUT;
	sprintf(mess->payload, "%s", current_user);
	mess->length = strlen(mess->payload);
	sendMessage(client_sock, *mess);
	receiveMessage(client_sock, mess);
	if (mess->type != TYPE_ERROR)
	{
		isOnline = 0;
		current_user[0] = '\0';
		requestId = 0;
	}
	// printf("%s\n", mess->payload);
	printf("LOGGED OUT SUCCESSFULLY!\n");
}

void menuAuthenticate()
{
	printf("\n------------------Storage System------------------\n");
	printf("\n1 - Login");
	printf("\n2 - Register");
	printf("\n3 - Exit");
	printf("\nChoose: ");
}

void mainMenu()
{
	printf("\n------------------Menu------------------\n");
	printf("\n1 - Upload file");
	printf("\n2 - Download File");
	printf("\n3 - Open folder");
	printf("\n4 - Create folder");
	printf("\n5 - Logout");
	printf("\nPlease choose: ");
}

void authenticateFunc()
{
	menuAuthenticate();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		loginFunc(current_user);
		break;
	case '2':
		registerFunc(current_user);
		break;
	case '3':
		exit(0);
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

void requestFileFunc()
{
	mainMenu();
	scanf(" %c", &choose);
	while (getchar() != '\n')
		;
	switch (choose)
	{
	case '1':
		uploadMultiFile();
		getDirectory();
		break;
	case '2':
		downloadFile();
		break;
	case '3':
		openFolder(root);
		break;
	case '4':
		createNewFolder();
		getDirectory();
		break;
	case '5':
		logoutFunc(current_user);
		break;
	default:
		printf("Syntax Error! Please choose again!\n");
	}
}

// communicate from client to server
// send and recv message with server
void communicateWithUser()
{
	while (1)
	{
		if (!isOnline)
		{
			authenticateFunc();
		}
		else
		{
			requestFileFunc();
		}
	}
}

int main(int argc, char const *argv[])
{
	// check valid of IP and port number
	if (argc != 3)
	{
		printf("Error!\nPlease enter two parameter as IPAddress and port number!\n");
		exit(0);
	}
	char *serAddr = malloc(sizeof(argv[1]) * strlen(argv[1]));
	strcpy(serAddr, argv[1]);
	int port = atoi(argv[2]);
	mess = (Message *)malloc(sizeof(Message));
	mess->requestId = 0;
	if (!validPortNumber(port))
	{
		perror("Invalid Port Number!\n");
		exit(0);
	}
	if (!checkIP(serAddr))
	{
		printf("Invalid Ip Address!\n"); // Check valid Ip Address
		exit(0);
	}
	strcpy(serAddr, argv[1]);
	if (!hasIPAddress(serAddr))
	{
		printf("Not found information Of IP Address [%s]\n", serAddr); // Find Ip Address
		exit(0);
	}

	// Step 1: Construct socket
	client_sock = initSock();
	// Step 2: Specify server address

	bindClient(port, serAddr);

	// Step 3: Request to connect server
	connectToServer();

	// Step 4: Communicate with server
	communicateWithUser();

	close(client_sock);
	return 0;
}