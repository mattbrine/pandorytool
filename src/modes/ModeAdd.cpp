#include <iostream>
#include <filesystem>
#include <map>
#include <tinyxml2.h>
#include "ModeAdd.h"
#include "../McGamesXML.h"
#include "../McGamesTXT.h"

#include "../SystemMapper.h"

std::string ModeAdd::pad(std::string string, const size_t size, const char character = ' ')
{
    if(size > string.size()) {
        string.insert(0, size - string.size(), character);
    }
    return string;
}

bool ModeAdd::validate() {
    if (!Fs::exists(sourceDir)) {
        std::cout << sourceDir << " does not exist " << std::endl;
        return false;
    }
    if (!Fs::exists(targetDir)) {
        std::cout << targetDir << " does not exist " << std::endl;
        return false;
    }
    return true;
}

ModeAdd::ModeAdd(std::string &sourceDir, std::string &targetDir) : sourceDir(sourceDir), targetDir(targetDir) {

}

// 1. Add games (MCGames)
int ModeAdd::main() {
    if (!validate()) {
        return 1;
    }
    createTargetDirectory();
    resetInstallFile();
    resetMcGamesFolder();
    parseSourceDirectory();
    return 0;
}

void ModeAdd::parseSourceDirectory() {
    openInstallFileHandle();
    for (const auto &entry : filesystem::directory_iterator(this->sourceDir)) {
        std::string filePath = entry.path().string();
        std::string basename = Fs::basename(filePath);
        std::string gameListXml = filePath + "/gamelist.xml";
        if (Fs::exists(gameListXml)) {
            parseSourceGameXML(gameListXml);
        }
    }
    closeInstallFileHandle();
}

void ModeAdd::openInstallFileHandle()
{
    installFile.open(getInstallFilePath().c_str());
}

void ModeAdd::closeInstallFileHandle()
{
    installFile.close();
}

void ModeAdd::parseSourceGameXML(const std::string &gameListXml) {
    tinyxml2::XMLDocument doc;
    doc.LoadFile(gameListXml.c_str());
    std::string directory = Fs::dirname(gameListXml);
    tinyxml2::XMLElement *gameList = doc.FirstChildElement("gameList");
    tinyxml2::XMLElement *provider = gameList->FirstChildElement("provider");
    std::string system = Fs::basename(directory);
    int i = 1;
    for (tinyxml2::XMLElement *game = gameList->FirstChildElement("game");
         game != nullptr;
         game = game->NextSiblingElement("game")) {
        const char *romPath = game->FirstChildElement("path")->GetText();
        const char *romName = game->FirstChildElement("name")->GetText();
        std::string absoluteRomPath = directory + "/" + romPath;
        std::string shortSystemName = SystemMapper::convertDirectoryNameToSystemName(system);
        if (!shortSystemName.empty()) {
            std::string targetRomName = shortSystemName + pad(std::to_string(i), 4, '0');
            std::string targetRomDir = targetDir + "/mcgames/" + targetRomName;
            /*std::string floppy = "\U0001F4BE";
            std::string cdrom = "\U0001F4C0";
            int filesize = Fs::filesize(absoluteRomPath) / 1024 / 1024;
            std::string icon = (filesize <= 50) ? floppy : cdrom;
            if (getenv("COMSPEC") != nullptr) {
                std::string comspec = getenv("COMSPEC");
                if (comspec.find("cmd.exe") != std::string::npos) {
                    icon = "-";
                }
            }*/
            std::cout << "- Found "
                 << extractXMLText(provider->FirstChildElement("System"))
                 << " ROM: " << romName << " [ " << Fs::basename(romPath)
                 << " ]" << endl;

            if (!Fs::exists(targetRomDir)) {
                Fs::makeDirectory(targetRomDir);
            }
            copyRomToDestination(absoluteRomPath, targetRomDir);

            std::string videoPath;
            if (game->FirstChildElement("video") != nullptr) {
                videoPath = extractXMLText(game->FirstChildElement("video"));
                std::string absoluteVideoPath = directory;
                absoluteVideoPath += "/" + videoPath;
                copyRomVideoToDestination(absoluteVideoPath, targetRomDir);
            }
            installFile << targetRomName << std::endl;
            generateMcGamesMeta(game, shortSystemName, targetRomDir, targetRomName);
            i++;
        } else {
            cout << "Unknown system detected in source folder: " << system << endl;
        }
    }
}

// Checks subfolders within “source rom folder”.
// Copies both rom file
// – to usbdevice:\mcgames\DC0001 (if dreamcast game) – check “ARSENAME” below for naming convention.
// Both DC0001.cdi should be within the mcgames/DC0001 folder.
void ModeAdd::copyRomToDestination(const std::string &rom, const std::string &destination) {
    std::string basename = Fs::basename(destination);
    std::string extension = Fs::getExtension(rom);
    Fs::copy(rom, destination + "/" + basename + extension);
}

// – (and if available, mp4 file (from romsubfolder/media/videos))
// DC0001.mp4 should be within the mcgames/DC0001 folder.
void ModeAdd::copyRomVideoToDestination(const std::string &absoluteVideoPath, const std::string &destination)
{
    std::string basename = Fs::basename(destination);
    std::string extension = Fs::getExtension(absoluteVideoPath);
    Fs::copy(absoluteVideoPath, destination + "/" + basename + extension);
}

// remove install.txt file if exists, open clear file
void ModeAdd::resetInstallFile() {
    std::string mcInstall = getInstallFilePath();
    FILE *foo;
    foo = fopen(mcInstall.c_str(), "w");
    fclose(foo);
}

void ModeAdd::resetMcGamesFolder() {
    std::string mgGamesFolder = getMcGamesFolder();
    Fs::remove(getMcGamesFolder());
    Fs::makeDirectory(getMcGamesFolder());
}

string ModeAdd::getInstallFilePath() {
    string mcInstall = this->targetDir + "/install.txt";
    return mcInstall;
}

bool ModeAdd::createTargetDirectory() {
    string mc = getMcGamesFolder();
    bool result = false;
    if (!Fs::exists(mc)) {
        cout << "Making " << mc << endl;
        result = Fs::makeDirectory(mc);
        if (!result) {
            cout << "Cannot create target" << endl;
        }
    }
    return result;
}

string ModeAdd::getMcGamesFolder() {
    string mc = this->targetDir + "/mcgames";
    return mc;
}

std::string ModeAdd::extractXMLText(tinyxml2::XMLElement *elem)
{
    if (elem->GetText() != nullptr) {
        return elem->GetText();
    }
    return std::string();
}


// Template.txt and template.xml (from template file) should be then copied to the DC0001 folder with the
// following variables changed depending on system and game (see below)
// install.txt file should then be appended with the “ARSENAME” (DC0001)
// repeat / loop process until all roms have been added.
void ModeAdd::generateMcGamesMeta(tinyxml2::XMLElement *sourceGame, std::string shortSystemName, std::string romPath, std::string romName) {

    // emulator type check code definitely bullshit- cant get this if statement to work ;(
    int emutype=99;
    //test cout << "short system name is " << shortSystemName << endl;
	    	if (shortSystemName == "FBA") {
        	emutype = 1;}
		if (shortSystemName == "MAME37") {
		emutype = 2;}
		if (shortSystemName == "MAME139") {
		emutype = 3;}
		if (shortSystemName == "MAME78") {
		emutype = 4;}
		if (shortSystemName == "PSP") {
		emutype = 6;}
		if (shortSystemName == "PS") {
		emutype = 7;}
		if (shortSystemName == "N64") {
		emutype = 8;}
		if (shortSystemName == "NES") {
		emutype = 11;}
		if (shortSystemName == "SNES") {
		emutype = 12;}
		if (shortSystemName == "GBA") {
		emutype = 13;}
		if (shortSystemName == "GBC") {
		emutype = 14;}
		if (shortSystemName == "MD") {
		emutype = 15;}
		if (shortSystemName == "WSWAN") {
		emutype = 16;}
		if (shortSystemName == "PCE") {
		emutype = 17;}
		if (shortSystemName == "DC") {
		emutype = 18;}
		if (shortSystemName == "MAME19") {
		emutype = 19;}
	

    std::string emuString = pad(std::to_string(emutype), 3, '0');
    std::string desc = extractXMLText(sourceGame->FirstChildElement("desc"));
    std::string relativeRomPath = Fs::basename(extractXMLText(sourceGame->FirstChildElement("path")));
    std::string dateString = extractXMLText(sourceGame->FirstChildElement("releasedate"));
    int year = (!dateString.empty()) ? std::stoi(dateString.substr(0, 4)) : 0;
    std::string developer = extractXMLText(sourceGame->FirstChildElement("developer"));
    std::string targetXMLFile = romPath + "/" + romName + ".xml";
    std::string targetTXTFile = romPath + "/" + romName + ".txt";

    McGamesXML mcXML;
    mcXML.setEmulatorName(emuString);
    mcXML.setRomName(romName);
    mcXML.setRomDescription(desc);
    mcXML.setLanguage("EN"); //TODO is this always true?
    mcXML.setYear(year);
    mcXML.setRomDeveloper(developer);
    mcXML.setRomPath(relativeRomPath);
    mcXML.generate(targetXMLFile);

    McGamesTXT mcTXT;
    mcTXT.setEmulatorName(emuString);
    mcTXT.setRomName(romName);
    mcTXT.setRomDescription(desc);
    mcTXT.setLanguage("EN"); //TODO is this always true?
    mcTXT.setYear(year);
    mcTXT.setRomDeveloper(developer);
    mcTXT.setRomPath(relativeRomPath);
    mcTXT.generate(targetTXTFile);
}



