#include "metro.h"

#include <cstdio>
#include <exception>
#include <iostream>
#include <string>

using namespace std;

#ifndef METRO_NO_MAIN

static Metro* builtInSystem()
{
    Metro* metro = new Metro();
    metro->loadDefaultNetwork();
    return metro;
}

static Metro* freshSystem(const string& dataPath, const string& seedFolder)
{
    Metro* metro = new Metro();
    string error, summary;
    switch (loadStartingData(*metro, seedFolder, error, summary)) {
    case SeedStatus::Loaded:
        ui::success("Starting data loaded from " + seedFolder + "/: " + summary + ".");
        break;
    case SeedStatus::FolderMissing:
        ui::warn("No starting-data files in " + seedFolder + "/. Using the built-in demo network.");
        delete metro;
        metro = builtInSystem();
        break;
    case SeedStatus::Error:
        ui::error("Problem in the starting-data files: " + error);
        ui::warn("Using the built-in demo network for now. Fix the file, then delete " +
                 dataPath + " and start again.");
        delete metro;
        metro = builtInSystem();
        break;
    }

    if (!Storage::save(*metro, dataPath, error))
        ui::error("Could not create data file: " + error);
    metro->clearDirty();
    return metro;
}

int main(int argc, char** argv)
{
    string dataPath = "metro_data.txt";
    string seedFolder = "data";

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--data" && i + 1 < argc) {
            dataPath = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seedFolder = argv[++i];
        } else {
            cout << "Usage: " << argv[0]
                 << " [--data FILE] [--seed FOLDER]\n";
            return arg == "--help" ? 0 : 1;
        }
    }

    try {
        Metro* metro = new Metro();
        string error;
        switch (Storage::load(*metro, dataPath, error)) {
        case LoadStatus::Loaded:
            break;
        case LoadStatus::NotFound:
            ui::info("No saved data found (first run).");
            delete metro;
            metro = freshSystem(dataPath, seedFolder);
            break;
        case LoadStatus::Corrupt: {
            ui::error("Saved data is damaged: " + error);
            string backup = dataPath + ".corrupt";
            remove(backup.c_str());
            rename(dataPath.c_str(), backup.c_str());
            ui::warn("Damaged file kept as " + backup + ". Starting again from the starting data.");
            delete metro;
            metro = freshSystem(dataPath, seedFolder);
            break;
        }
        }

        App app(*metro, dataPath);
        try {
            app.run();
        } catch (const ui::InputClosed&) {
            string saveError;
            if (metro->isDirty() && !Storage::save(*metro, dataPath, saveError))
                ui::error("Save failed: " + saveError);
            cout << "\nInput closed. Data saved. Goodbye.\n";
        }
        delete metro;
    } catch (const exception& ex) {
        cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}

#endif
