#ifndef _logboek_input_h_
#define _logboek_input_h_

// definitions of layouts
#define LAYOUTFILE <Logboek/input.lay>
#include <CtrlCore/lay.h>


class InputDlg : public WithInputLayout<TopWindow> {
	typedef InputDlg CLASSNAME;
private:
	myStatusBar status;
	bool dirty;
	
	void UpdateFromAny();
	void UpdateFromDepartment();
	void UpdateFromLinenumber();
	void UpdateFromTime();
	void UpdateFromTotalTime();
	
	void UpdateAllLists();
	//void UpdateDepartmentList();
	void UpdateLinenumberList();
	void UpdateMachineList();
	
	void Reset();
	void ConfirmStore();
	void ConfirmClose();
	
public:
	InputDlg();
};

#endif
