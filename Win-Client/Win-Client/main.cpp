#define NOMINMAX
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <errno.h>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <direct.h>
#include <winsock2.h>
#include <Windows.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <QApplication>
#include "window.h"
#include <QWidget>

/**
* Funkcja sprawdza czy plik jest plikiem zwyk³ym
*
* @param path œcie¿ka do pliku
* @return wartoœæ true lub false zale¿na od wyniku operacji
*/

int is_regular(const char *path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	return (path_stat.st_mode & S_IFREG);
}

/**
* Funkcja wysy³a dane przez internet
*
* @param sock deskryptor po³¹czenia
* @param buf wskaŸnik do miejsca w pamiêci w którym znajduj¹ siê dane
* @param buflen iloœæ danych w bajtach
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
*/


bool senddata(int sock, void *buf, int buflen)
{
	unsigned char *pbuf = (unsigned char *)buf;
	cout << "STOP" << endl;
	while (buflen > 0) //póki nie wys³ano wszystkiego z bufora
	{
		int num = send(sock, (const char*)pbuf, buflen, 0); //spróbuj wys³aæ wszystko i zapisz ile uda³o siê wys³aæ
		if (num == -1)
		{
			if (errno == EWOULDBLOCK)
			{
				// optional: use select() to check for timeout to fail the send
				continue;
			}
			return false;
		}

		pbuf += num; //przesuñ wskaŸnik o num pozycji
		buflen -= num;//zmieñ pozosta³¹ iloœæ bajtów
	}

	return true;
}

/**
* Wysy³a liczbê u¿ywaj¹c funkcji senddata
*
* @param sock deskryptor po³¹cznia
* @param value wartoœæ do wys³ania
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
* @see senddata
*/

bool sendlong(int sock, long long value)
{
	cout << "STOP" << endl;
	value = htonl(value);
	cout << "STOP" << endl;
	return senddata(sock, &value, sizeof(value));
}

/**
* Wysy³a nazwê pliku przez internet
* analogiczna do sendfile tylko dla tablicy zapisanej w pamiêci
*
* @param sock deskryptor po³¹czenia
* @param path nazwa pliku
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
* @see sendlong
* @see senddata
* @see sendfile
*/

bool sendpath(int sock, std::string path) {
	long namesize = (long)path.length();
	long position = 0;
	if (!sendlong(sock, namesize)) {
		return false;
	}
	if (namesize > 0) {
		char buffer[1024];
		do
		{
			size_t num = std::min((size_t)namesize, sizeof(buffer));
			printf("%s\n", path.c_str());
			memcpy(buffer, path.substr(position, num).c_str(), num + 1);
			senddata(sock, buffer, num);
			namesize -= num;
			position += num;
		} while (namesize > 0);
	}
	else
		return false;
	return true;

}

/**
* Wysy³a plik przez internet
*
* @param sock deskryptor po³¹czenia
* @param path scie¿ka do pliku
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
* @see senddata
* @see sendlong
*/

bool sendfile(int sock, std::string path)
{
	FILE *f = fopen(path.c_str(), "rb");//otwarcie pliku do odczytu
	if (f == NULL) {//je¿eli siê nie uda³o
		perror("Failed to open file");//to wypisz czemu
		return false;//i zakoñcz
	}
	long mode = 2;
	sendlong(sock, mode);
	fseek(f, 0, SEEK_END);//znajdz koniec
	long filesize = ftell(f);//sprawdŸ rozmiar
	rewind(f);//wróæ na pocz¹tek
	string temp = path;
	for (int i = temp.length(); i > 0; i--) {
		if (temp[i] == '/') {
			temp.erase(0, i + 1);
			break;
		}
	}
	sendpath(sock, temp);
	if (filesize == EOF)//je¿eli rozmiar = -1
		return false;// to zakoñcz z b³êdem
	if (!sendlong(sock, filesize))//wyœlij rozmiar i je¿eli siê nie uda³o
		return false;//zakoñcz z b³êdem
	if (filesize > 0)//je¿eli plik jest wiêkszy ni¿ 0
	{
		char buffer[1024];//stwórz buffor

		do
		{
			size_t num = (std::min)((size_t)filesize, sizeof(buffer));//wybierz mniejsze rozmiar buffor czy pozosta³y rozmiar pliku do wys³ania
			num = fread(buffer, 1, num, f);//przeczytaj num znaków o wielkoœci 1 bajt do buffer z pliku f i przypisz ile uda³o siê odczytaæ do num
			if (num < 1)
				return false;
			if (!senddata(sock, buffer, num))//wyœlij num znaków z buffer
				return false;
			filesize -= num;//zmniejsz pozosta³y rozmiar pliku o iloœæ wys³anych bajtów
		} while (filesize > 0);//póki coœ zosta³o do wys³ania
	}

	fclose(f);//zamknij plik
	return true;
}

/**
* Wysy³a katalog przez internet
*
* @param sock deskryptor katalogu
* @param path scie¿ka do katalogu
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
* @see sendfile
* @see sendpath
*/



bool senddirectory(int sock, string path) {
	QDir directory(path.c_str()); //otwarcie katalogu jako strumieñ
	long mode = 1; //zmienna odpowiadaj¹ca za typ pliku do wys³ania
	QFileInfoList list = directory.entryInfoList();

	for (int i = 0; i < list.size(); ++i) {
		QFileInfo file = list.at(i);
		string name = file.fileName().toStdString(); //przypisanie nazwy pliku do name
		if (!is_regular((path + "/" + name).c_str()))//je¿eli katalog
		{
			if (strcmp(name.c_str(), "."))//je¿eli nie ten sam katalog
			{
				if (strcmp(name.c_str(), ".."))//je¿eli nie katalog wczeœniejszy
				{
					mode = 1; //1 w kliencie odpowiada za odebranie katalogu
					if (!sendlong(sock, mode)) {//wys³anie trybu odczytu
						return false;
					}
					sendpath(sock, name); //wys³anie nazwy katalogu
					senddirectory(sock, path + "/" + name);//otwarcie katalogu aby mo¿na by³o go wys³aæ
				}
			}
		}
		else {//je¿eli plik
			  /*	mode = 2;//odpowiada za czytanie pliku w kliencie
			  sendlong(sock, mode);//wyslanie trybu
			  sendpath(sock, name);//wyslanie nazwy pliku */
			sendfile(sock, (path + "/" + name));//wys³anie pliku
		}
		//odczytanie nastêpnego pliku w katalogu
	}
	sendlong(sock, long long(0));//wyslanie trybu zakoñczenia katalogu
	return true;
}

/**
* Funkcja odczytuje dane z internetu
*
* @param sock deskryptor po³¹czenia
* @param buf wskaŸnik do miejsca w pamiêci w którym zapisane zostan¹ dane
* @param buflen iloœæ danych w bajtach
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
*/
bool readdata(int sock, void *buf, int buflen)
{
	unsigned char *pbuf = (unsigned char *)buf;
	while (buflen > 0)
	{
		int num = recv(sock, (char *)pbuf, buflen, 0);
		if (num == -1)
		{
			if (errno == EWOULDBLOCK)
			{
				// optional: use select() to check for timeout to fail the read
				continue;
			}
			return false;
		}
		if (num == 0)
			return false;

		pbuf += num;
		buflen -= num;
	}

	return true;
}

/**
* odczytuje zmienna typu long z internetu
* @param sock deskryptor po³¹czenia
* @param value wskaŸnik na zmien¹ do której zapisana zostanie wartoœæ
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
*/

bool readlong(int sock, long *value)
{
	if (!readdata(sock, value, sizeof(value)))
		return false;
	*value = ntohl(*value);
	return true;
}

/**
* odczytujê nazwê pliku
* @param sock deskryptor po³¹czenia
* @return zwraca (string) z nazw¹
* @see readlong
*/

std::string readpath(int sock)
{
	long namesize = 0;
	std::string path = "";
	if (!readlong(sock, &namesize)) {//odczytanie d³ugoœci nazwy
		return NULL;
	}
	if (namesize > 0) {//je¿eli d³ugoœæ jest wiêksza ni¿ 0
		char buffer[1024];
		do
		{
			size_t num = (std::min)((size_t)namesize, sizeof(buffer));//wybierz mniejsze rozmiar nazwy lub bufora
			if (!readdata(sock, buffer, num))//odczytaj dane
				return NULL;
			for (unsigned int i = 0; i<num; i++)//przepisz buffor do pliku znak po znaku
				path += buffer[i];
			namesize -= num; //zmniejsz pozosta³y rozmiar nazwy pliku o num
		} while (namesize > 0);//do czasu a¿ rozmiar nazwy pliku
	}
	else
		return NULL;
	return path;

}

/**
* odczytuje plik z internetu
* @param sock deskryptor po³¹czenia
* @param f desktryptor pliku
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
* @see readlong
*/

bool readfile(int sock, FILE *f)
{
	long filesize = 0;
	if (!readlong(sock, &filesize)) //odczytaj rozmiar pliku
		return false;
	int progress = 0;
	int maxsize = filesize;
	if (filesize > 0)
	{
		char buffer[1024];
		do
		{
			size_t num = (std::min)((size_t)filesize, sizeof(buffer));
			if (!readdata(sock, buffer, num)) //odczytaj num danych
				return false;
			size_t offset = 0;
			do
			{
				size_t written = fwrite(&buffer[offset], 1, num - offset, f); //przepisz je do pliku
				if (written < 1)
					return false;
				offset += written; //zmieñ pozycjê kursora w pamiêci
			} while (offset < num); //powtarzaj dopóki nie przepiszesz wszystkiego co odebra³eœ
			filesize -= num;//zmienjsz pozosta³y rozmiar pliku
			progress += num;
			printf("%d / %d\n", progress, maxsize); //wypisywanie ile uda³o siê odebraæ
		} while (filesize > 0);
	}
	return true;
}

/**
* odbiera strukturê katalogów wraz z plikami
* @param sock desktryptor po³¹czenia
* @param path scie¿ka gdzie odebraæ "." dla aktualnego katalogu
* @return zwraca true je¿eli sie uda³o lub false je¿eli nie
*/

bool recvdirectory(int sock, std::string path) {
	FILE *filehandle;
	long mode = 0;
	std::string name;
	do
	{
		if (!readlong(sock, &mode)) { //czytanie trybu
			return false;
		}
		if (mode != 0)
		{
			name = readpath(sock);//odczytaj nazwê pliku/katalogu
		}
		if (mode == 1) {//odbierz katalog
			_mkdir((path + "/" + name).c_str()); //stwórz katalog
			recvdirectory(sock, (path + "/" + name)); //wejdz do katalogu aby móc go odczytaæ
		}
		else if (mode == 2) {//odbierz plik
			filehandle = fopen((path + "/" + name).c_str(), "wb");
			readfile(sock, filehandle);
			fclose(filehandle);

		}
	} while (mode);
	return true;
}

int main(int argc, char* argv[]) {


	QApplication app(argc, argv);

	Window window;
	window.show();

	app.exec();
	window.show();
	WORD WRequiredVersion;
	WSADATA WData;
	SOCKET SSocket;

	WRequiredVersion = MAKEWORD(2, 0);
	if (WSAStartup(WRequiredVersion, &WData) != 0) {
		fprintf(stderr, "WSAStartup failed!");
		exit(1);
	}



	int sfd;
	struct sockaddr_in saddr;
	struct hostent* addrent;
	bool ok;
	addrent = gethostbyname(window.hostName.c_str());
	sfd = socket(PF_INET, SOCK_STREAM, 0);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(window.ipAdd.c_str()));
	memcpy(&saddr.sin_addr.s_addr, addrent->h_addr, addrent->h_length);

	if (connect(sfd, (struct sockaddr*) &saddr, sizeof(saddr)) == -1) {
		std::cout << "Blad laczenia" << endl;
		perror("Failed to connect");
		return 1;
	}
	printf("Connected to %s:%s\n", window.hostName.c_str(), window.ipAdd.c_str());
	std::string path = "Odebrane_pliki";
	_mkdir(path.c_str());
	std::string s_path = window.filePath;
	if (!is_regular(s_path.c_str()))//je¿eli katalog
	{
		if (senddirectory(sfd, s_path.c_str()))//zamiast string sciezka do pliku
		{
			printf("Wyslano pliki bez problemów\n");
		}

	}
	else
	{
		sendfile(sfd, s_path.c_str());//zamiast string sciezka do pliku
		sendlong(sfd, long long(0));
	}

	ok = recvdirectory(sfd, path);
	if (ok)
	{
		printf("Udalo sie odczytac pliki\n");
	}
	else
		remove("imagefile.jpg");
	closesocket(sfd);
	return 0;
}

