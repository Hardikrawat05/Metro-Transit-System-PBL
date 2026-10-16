#ifndef STUDENTREGISTRY_H
#define STUDENTREGISTRY_H

#include "common.h"

class StudentRegistry {
private:
    int studentIDs[100];
    bool active[100];
    int count;

public:
    StudentRegistry();
    bool registerStudent(int id);
    bool isValidStudent(int id);
    bool deactivateStudent(int id);
    void displayStudents();
};

