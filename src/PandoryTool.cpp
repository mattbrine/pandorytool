#include "CommandLineArguments.h"
#include "PandoryTool.h"
#include "modes/ModeAdd.h"
#include "modes/ModePrepare.h"
#include "modes/ModePspfix.h"
#include "modes/ModeStick.h"

PandoryTool::PandoryTool(int i, char **pString) {
    args = CommandLineArguments(i, pString);
}

std::string PandoryTool::getCommitHash() {
    std::string version = GIT_COMMIT_HASH;
    if (version.empty()) {
        return "000000";
    }
    return version;
}

std::string PandoryTool::getCompileDate() {
    std::string compileDate = GIT_COMPILE_DATE;
    return compileDate;
}


std::string PandoryTool::getAppSuffix() {
    std::string suffix = " [Full Edition]";
#ifndef NO_SHAREWARE_LIMIT
    suffix = " [Standard Edition]";
#endif
    return suffix;
}

int PandoryTool::main() {
    std::cout << "-----------------------------------------------------------------------------------------------------" << std::endl;
    std::cout << "Pandora Games 3D - Utility" << getAppSuffix() << " by emuchicken & dajoho      [Version: " << getCompileDate() << "/" << getCommitHash() << "]" << std::endl;
    std::cout << "== Join us on Discord! https://discord.gg/Az94Rxn" << std::endl;
    std::cout << "== Like us on Facebook! https://www.facebook.com/groups/2522039741415070/" << std::endl;
    std::cout << "== Visit us on PG3D-HAX! https://pg3d-hax.uk" << std::endl;
    std::cout << std::endl;

#ifndef NO_SHAREWARE_LIMIT
    std::cout << "== Pandorytool has been worked on with blood, sweat and tears. If you would like us to continue " << std::endl;
    std::cout << "== work on this tool, please consider grabbing us a coffee at https://www.buymeacoffee.com/CKZbiXa." << std::endl;
    std::cout << std::endl;
    std::cout << " * Various MAME cfgs contributed by the community (check each cfg for credits)" << std::endl;
#endif
    std::cout << "-----------------------------------------------------------------------------------------------------" << std::endl;

    if (args.getArgumentCount() == 1) {
        usage();
        return 1;
    }
    std::string mode = args.getArgument(1);
    if (mode == "add") {
        return add();
    }

    if (mode == "prepare") {
        return prepare();
    }

    if (mode == "pspstockfix") {
        return pspStockfix();
    }

    if (mode == "pspfix") {
        return pspFix();
    }

    if (mode == "stick") {
        return stick();
    }

    return 0;
}

void PandoryTool::usage() {
    std::cout << "Usage: " << std::endl;

    //std::cout << "1) Create a folder with the correct structure for your ROMS:" << std::endl;
    //std::cout << "\t\tpandory prepare <sourceDir>" << std::endl;
    //std::cout << "\t\tExample: pandory prepare C:\\roms" << std::endl;
    //std::cout << std::endl;

    std::cout << "1) Put your ROMS in the correct subfolders and scrape them with Skraper:" << std::endl;
    std::cout << "   == Tutorial video [Using Skraper]     : https://www.youtube.com/watch?v=10naz1ZUOC4" << std::endl;
    std::cout << "   == Tutorial video [Advanced scraping] : https://www.youtube.com/watch?v=10naz1ZUOC4" << std::endl;
    std::cout << std::endl;

    std::cout << "2) => Take all the scraped ROMS from <sourceDir> and prepare them for your Pandora console in <destDir>" << std::endl;
    std::cout << "\t\tpandory add <sourceDir> <destDir>" << std::endl;
    std::cout << "\t\tExample: pandory add C:\\roms F:\\" << std::endl;
    std::cout << std::endl;

    std::cout << "-----------------------------------------------------------------------------------------------------" << std::endl;

    std::cout << "PANDORY JAILBREAK! - PLEASE BACK UP YOUR SYSTEM FIRST!" << std::endl;
    std::cout << "   == Tutorial video [Pandora MicroSD backup]: https://www.youtube.com/watch?v=sOluaeOtTX4" << std::endl;
    std::cout << "   == Tutorial video [Full system backups]   : https://www.youtube.com/watch?v=HdLaEymH0ok" << std::endl;
    std::cout << "   == Tutorial video [How to restore backups]: https://www.youtube.com/watch?v=02VBVWCTXgc" << std::endl << std::endl;

    std::cout << "3) => Create a `Backup-to-USB`-Stick that will backup your system/data partitions before jailbreaking" << std::endl;
    std::cout << "\t\tpandory stick backup <destDrive>" << std::endl;
    std::cout << "\t\tExample: pandory stick backup F:\\" << std::endl;
    std::cout << std::endl;

    std::cout << "4) => Create a `Jailbreak`-Stick that will patch your system and allow you to run any compatible" << std::endl;
    std::cout << "      application from USB." << std::endl;
    std::cout << "\t\tpandory stick jailbreak <destDrive>" << std::endl;
    std::cout << "\t\tExample: pandory stick jailbreak F:\\" << std::endl;
    std::cout << std::endl;

    std::cout << "-----------------------------------------------------------------------------------------------------" << std::endl;

    std::cout << "FIX PSP CONTROLS & TWO-PLAYER MODES!" << std::endl;
    std::cout << "5) => Fix controls for inbuilt (stock) or pandory-added games" << std::endl;
    std::cout << "\t\tpandory pspfix stage1 <destDrive>" << std::endl;
    std::cout << "\t\tpandory pspfix stock <destDrive>" << std::endl;
    std::cout << "\t\tpandory pspfix other <destDrive>" << std::endl;
    std::cout << "\t\tExample: pandory pspstockfix F:\\" << std::endl;
    std::cout << "\t\tExample: pandory pspfix F:\\" << std::endl;
    std::cout << std::endl;
}

int PandoryTool::add() {
    std::string sourceDir = args.getArgument(2);
    std::string targetDir = args.getArgument(3);
    if (sourceDir.empty() || targetDir.empty()) {
        usage();
        return 1;
    }
    ModeAdd add(sourceDir, targetDir);
    return add.main();
}

int PandoryTool::prepare() {
    std::string sourceDir = args.getArgument(2);
    if (sourceDir.empty()) {
        usage();
        return 1;
    }
    ModePrepare prepare(sourceDir);
    return prepare.main();
}


int PandoryTool::pspStockfix() {
    std::string targetDir = args.getArgument(2);
    if (targetDir.empty()) {
        usage();
        return 1;
    }
    ModePspfix pspfix(targetDir);
    return pspfix.stage2();
}

int PandoryTool::pspFix() {
    std::string subMode = args.getArgument(2);
    std::string targetDir = args.getArgument(3);
    if (targetDir.empty() || subMode.empty()) {
        usage();
        return 1;
    }
    ModePspfix pspfix(targetDir);

    if (subMode == "stock") {
        return pspfix.stage2();
    }
    else if (subMode == "other") {
        return pspfix.otherFix();
    }
    else if (subMode == "stage1") {
        return pspfix.stage1();
    }
    usage();
    return 1;
}




int PandoryTool::stick() {
    std::string stickType = args.getArgument(2);
    std::string target = args.getArgument(3);
    if (stickType.empty() || target.empty()) {
        usage();
        return 1;
    }
    ModeStick stick(stickType, target);
    stick.main();
    return 0;
}

