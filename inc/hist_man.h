/* this is the histogram manager. The user code can use this to get the
 * histogram by id or name. 
 *
 * By B.Gao Jan. 2019 */
#ifndef HIST_MAN_HHH
#define HIST_MAN_HHH

#include <vector>
#include <TObject.h>
#include <string>

struct hist_pars
{
	TObject* p_obj;
	int id;
	std::string type;
	std::string name;
	std::string folder;
	int nbinsX;
	int nbinsY;
	double X_min;
	double Y_min;
	double X_max;
	double Y_max;
};

class hist_man
{
public:
	hist_man(){};
	~hist_man(){};

	TObject* get(int id)
	{
		for (auto it = objs.begin(); it != objs.end(); it++) {
			if ((*it).id == id)
				return (*it).p_obj;
		}
		return NULL;
	}

	TObject* get(const char* name)
	{
		for (auto it = objs.begin(); it != objs.end(); it++) {
			if ((*it).name == name)
				return (*it).p_obj;
		}
		return NULL;
	}

	void push(hist_pars& obj) {objs.push_back(obj);}

private:
	std::vector<struct hist_pars> objs;
};

#endif 
