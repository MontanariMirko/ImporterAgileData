/***************************************************************************
                          main.cpp
                             -------------------
    copyright            : (C) 2013 Montanati, Paganelli, Bulgarelli
    email                : bulgarelli@iasfbo.inaf.it, ,

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Freeze/Freeze.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "InputFileFITS.h"
#include "MapAgileEvt.h"
#include "MapAgileLog.h"

//file types
#define EVT 1
#define LOG 2

//columns of EVT file
#define EVT_TIME 0
#define EVT_ENERGY 5
#define EVT_PH_EARTH 14
#define EVT_THETA 1
#define EVT_PHASE 15
#define EVT_EVSTATUS 13

//columsn of LOG file
#define LOG_TIME 0
#define LOG_PHASE 5
#define LOG_LIVETIME 39
#define LOG_LOG_STATUS 40
#define LOG_MODE 4
#define LOG_ATTITUDE_RA_Y 13
#define LOG_ATTITUDE_DEC_Y 14
#define LOG_EARTH_RA 24
#define LOG_EARTH_DEC 25
#define LOG_Q1 9
#define LOG_Q2 10
#define LOG_Q3 11
#define LOG_Q4 12

using namespace std;
using namespace qlbase;

///Import AGILE LOG and EVT files into Ice/Freeze/BDB
int main(int argc, char** argv) {

	if(argc == 1) {
		cerr << "Please, provide (1) the fits file to import (2 optional) the last line to be read" << endl;
		return 0;
	}

	string filename = argv[1];
	InputFileFITS* inputFF;

	//set properties for db ice
	Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
	initData.properties->load("config");

	//initialize che Ice communicator
	Ice::CommunicatorPtr communicator = Ice::initialize(argc,argv,initData);

	//create the Freeze's db with the BDB in disk
	Freeze::ConnectionPtr connection = Freeze::createConnection(communicator, "db");

	//inizialize the maps
	MapAgileEvt mapEvt(connection, "AgileEvt");
	MapAgileLog mapLog(connection, "AgileLog");

	//open input file FITS
	try{
		inputFF = new InputFileFITS();
		inputFF->open(filename);
		inputFF->moveToHeader(1); //CHECK base index AZ


		//check input file fits type: EVT or LOG
		int ncols = inputFF->getNCols();
		int nrows_start = 0;
		int nrows_end = inputFF->getNRows();
		if(argc == 3) {
			nrows_end = atoi(argv[2]);
		}
		int type;

		if(ncols == 19) type = EVT;
		if(ncols == 41) type = LOG;

		if(type == EVT) {
			//read all columns
			cout << "Read EVT file " << endl;
			std::vector<double> time = inputFF->read64f(EVT_TIME, nrows_start, nrows_end-1);
			std::vector<float> energy = inputFF->read32f(EVT_ENERGY, nrows_start, nrows_end-1);
			std::vector<float> ph_earth = inputFF->read32f(EVT_PH_EARTH, nrows_start, nrows_end-1);
			std::vector<float> theta = inputFF->read32f(EVT_THETA, nrows_start, nrows_end-1);
			std::vector<int16_t> phase = inputFF->read16i(EVT_PHASE, nrows_start, nrows_end-1);
			std::vector< std::vector<char> > status = inputFF->readString(EVT_EVSTATUS, nrows_start, nrows_end-1, 1);
			std::vector<int16_t> status2(nrows_end-nrows_start+1);

			for(uint32_t i  = 0; i<nrows_end; i++) {
				std::string evt;
				evt = &(status[i])[0];
				cout << evt << endl;
				if(evt.compare("G") == 0) status2[i] = 0;
				if(evt.compare("L") == 0) status2[i] = 1;
				if(evt.compare("S") == 0) status2[i] = 2;

			}
			//write data into BDB
			for(uint32_t i  = 0; i<nrows_end; i++) {
				//&(status[i])[0]
				cout << setiosflags(ios::fixed) << std::setprecision(6) << (double) time[i] << " " << status2[i] << endl;
				//insert data into BDB
				Struttura::agileEvt dataEvt;
				dataEvt[0] = time[i];
				dataEvt[1] = energy[i];
				dataEvt[2] = ph_earth[i];
				dataEvt[3] = theta[i];
				dataEvt[4] = phase[i];
				mapEvt.insert(make_pair(time[i],dataEvt));
			}
		}
		if(type == LOG) {
			//read all columns
			cout << "Read LOG file " << nrows_end << endl;
			std::vector<double> time = inputFF->read64f(LOG_TIME, nrows_start, nrows_end-1);
			std::vector<int16_t> phase = inputFF->read16i(LOG_PHASE, nrows_start, nrows_end-1);
			std::vector<float> livetime = inputFF->read32f(LOG_LIVETIME, nrows_start, nrows_end-1);
			std::vector<int16_t> log_status = inputFF->read16i(LOG_LOG_STATUS, nrows_start, nrows_end-1);
			std::vector<int16_t> mode = inputFF->read16i(LOG_MODE, nrows_start, nrows_end-1);
			std::vector<double> attitude_ra_y = inputFF->read64f(LOG_ATTITUDE_RA_Y, nrows_start, nrows_end-1);
			std::vector<double> attitude_dec_y = inputFF->read64f(LOG_ATTITUDE_RA_Y, nrows_start, nrows_end-1);
			std::vector<double> log_earth_ra = inputFF->read64f(LOG_EARTH_RA, nrows_start, nrows_end-1);
			std::vector<double> log_earth_dec = inputFF->read64f(LOG_EARTH_DEC, nrows_start, nrows_end-1);
			std::vector<float> q1 = inputFF->read32f(LOG_Q1, nrows_start, nrows_end-1);
			std::vector<float> q2 = inputFF->read32f(LOG_Q2, nrows_start, nrows_end-1);
			std::vector<float> q3 = inputFF->read32f(LOG_Q3, nrows_start, nrows_end-1);
			std::vector<float> q4 = inputFF->read32f(LOG_Q4, nrows_start, nrows_end-1);
			//write data into BDB
			uint32_t count = 0;
			for(uint32_t i = 0; i<nrows_end; i++) {
				cout << setiosflags(ios::fixed) << std::setprecision(6) << (double) time[i] << " ";
				cout << attitude_ra_y[i] << endl;
				if(attitude_ra_y[i] != 0) count++;
				//insert log data into BDB
				Struttura::agileLog dataLog;
				dataLog[0] = time[i];
				dataLog[1] = phase[i];
				dataLog[2] = livetime[i];
				dataLog[3] = log_status[i];
				dataLog[4] = mode[i];
				dataLog[5] = attitude_ra_y[i];
				dataLog[6] = attitude_dec_y[i];
				dataLog[7] = log_earth_ra[i];
				dataLog[8] = log_earth_dec[i];
				dataLog[9] = q1[i];
				dataLog[10] = q2[i];
				dataLog[11] = q3[i];
				dataLog[12] = q4[i];
				mapLog.insert(make_pair(time[i],dataLog));

			}
			cout << count << endl;
		}
	} catch(IOException* e) {
		cout << e->getErrorCode() << ": " << e->what() << endl;
		return e->getErrorCode();
	}
}
