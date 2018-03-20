#include "main.h"


using namespace std;


template<typename T>
T getNumFromString(const string &str, size_t &start_pos, const char terminate) {
	T num = 0;

	while ((start_pos < str.size()) && (str.at(start_pos) != terminate)) {
		// номер ascii символа '0' = 48
		num = num * 10 + str.at(start_pos++) - 48;
	}

	return num;
}


// работает только с беззнаковыми типами. Хз, почему.
template<typename T>
T fromString(const string &str) {
	T val;
	istringstream ss(str);

	ss >> val;

	return val;
}


// весь код поогнан под специфику файлов, с которыми он работает.
// так что не следует их редактировать без понимания структуры.
uint32_t findAbsPop(const uint32_t id1, const uint32_t id2, const string &str) {
	vector<SPoint<uint32_t, uint32_t>> abs_pop{ {0, id1}, {0, id2} };

	for (auto &&i : abs_pop) {
		size_t pos = str.find("\n" + to_string(i.y) + " ");

		if (string::npos != pos) {
			while ((pos < str.size()) && (str.at(pos++) != ' '));

			i.x = getNumFromString<uint32_t>(str, pos, '\n');
		}
	}

	// чтобы избежать потом деления на 0. А то мало ли что.
	return (abs_pop.at(0).x + abs_pop.at(1).x) ?
		(abs_pop.at(0).x + abs_pop.at(1).x) :
		1;
}


void getSimilarCharacters(const vector<uint32_t> ids, const uint32_t limit) {
	vector<SPoint<uint32_t, double>> tot_sim;
	for (auto id : ids) {
		vector<SPoint<uint32_t, double>> sim;
		// магических констант пора
		ifstream h_pop("popres1.txt"), h_tab("popres2.txt");

		if ((h_pop.is_open()) && (h_tab.is_open())) {
			string abs_pop = "";

			while (!h_pop.eof()) {
				string line;

				getline(h_pop, line);
				abs_pop.append(line + '\n');
			}
			h_pop.close();

			bool get_out = false;

			while ((!h_tab.eof()) && (!get_out)) {
				string line;

				getline(h_tab, line);

				size_t pos = 0;
				uint32_t ch_id = getNumFromString<uint32_t>(line, pos, ':');

				if (ch_id > id) {
					get_out = true;
				}
				else if (ch_id < id) {
					pos = line.find("{" + to_string(id) + ",");
					if (string::npos != pos) {
						while ((pos < line.size()) && (line.at(pos++) != ','));

						uint32_t rel_pop = getNumFromString<uint32_t>(line, pos, '}');

						if (rel_pop > 5) {
							SPoint<uint32_t, double> new_one;

							new_one.x = ch_id;
							new_one.y = static_cast<double>(rel_pop) /
								static_cast<double>(
									findAbsPop(id, ch_id, abs_pop) - rel_pop
								);

							if (new_one.y > 0.01) {
								new_one.y *= log(rel_pop);
								sim.push_back(new_one);
							}
						}
					}
				}
				else if (ch_id == id) {
					pos = 0;
					while (string::npos != pos) {
						// немного видоизменённая копипаста
						pos = line.find("{", pos);
						if (string::npos != pos) {
							ch_id = getNumFromString<uint32_t>(line, ++pos, ',');

							uint32_t rel_pop = getNumFromString<uint32_t>(line, ++pos, '}');

							if (rel_pop > 5) {
								SPoint<uint32_t, double> new_one;

								new_one.x = ch_id;
								new_one.y = static_cast<double>(rel_pop) /
									static_cast<double>(
										findAbsPop(id, ch_id, abs_pop) - rel_pop
									);

								if (new_one.y > 0.01) {
									new_one.y *= log(rel_pop);
									sim.push_back(new_one);
								}
							}
						}
					}
				}
			}
			h_tab.close();

			for (auto &&i : sim) {
				bool find = false;

				for (auto &&j : tot_sim) {
					if (i.x == j.x) {
						find = true;
						j.y += i.y;
					}
				}
				if (!find) {
					tot_sim.push_back(i);
				}
			}
		}
		else {
			cout << "error: can't open files" << endl;
			return;
		}
	}

	ifstream h_animes("animes.txt");
	vector<SPoint<uint32_t, vector<uint32_t>>> animes;

	if (h_animes.is_open()) {
		while (!h_animes.eof()) {
			string line;

			getline(h_animes, line);
			if (line != "") {
				if (string::npos != line.find("[")) {
					line.erase(0, 1);
					line.erase(line.size() - 1, 1);
					if (!h_animes.eof()) {
						animes.push_back(SPoint<uint32_t, vector<uint32_t>>());
						animes.at(animes.size() - 1).x = fromString<uint32_t>(line);

						uint32_t an_id = 0;
						size_t pos = 0;

						getline(h_animes, line);
						while (an_id = getNumFromString<uint32_t>(line, pos, ',')) {
							animes.at(animes.size() - 1).y.push_back(an_id);
							pos++;
						}
					}
				}
			}
		}
		h_animes.close();
	}

	std::sort(tot_sim.begin(), tot_sim.end(), [](
		SPoint<uint32_t, double> &a,
		SPoint<uint32_t, double> &b) {
		return a.y > b.y;
	}
	);

	cout << "[characters ids=";
	for (size_t id1 = 0, cnt = 0; id1 < tot_sim.size() && cnt < limit; ++id1) {
		size_t an_id1 = animes.size();

		for (size_t i = 0; i < animes.size(); ++i) {
			if (animes.at(i).x == tot_sim.at(id1).x) {
				an_id1 = i;
			}
		}

		bool from_same_title = false;

		for (size_t id2 = 0; id2 < ids.size(); ++id2) {
			if (ids.at(id2) == tot_sim.at(id1).x) {
				from_same_title = true;
			}

			size_t an_id2 = animes.size();

			for (size_t i = 0; i < animes.size(); ++i) {
				if (animes.at(i).x == ids.at(id2)) {
					an_id2 = i;
				}
			}

			if ((an_id1 < animes.size()) && (an_id2 < animes.size())) {
				for (auto v1 : animes.at(an_id1).y) {
					for (auto v2 : animes.at(an_id2).y) {
						if (v1 == v2) {
							from_same_title = true;
						}
					}
				}
			}
		}

		if (!from_same_title) {
			cout << tot_sim.at(id1).x << ",";
			cnt++;
		}
	}
	cout << " columns=9]" << endl;
}


int main(int argc, char** argv) {
	vector<uint32_t> ids;

	for (auto i = 1; i < argc - 1; ++i) {
		ids.push_back(fromString<uint32_t>(argv[i]));
	}

	uint32_t limit = fromString<uint32_t>(argv[argc - 1]);

	if (argc < 3) {
		cout << "enter number of characters: ";
		cin >> argc;
		cout << endl << "enter characters id: ";
		for (auto i = 0; i < argc; ++i) {
			uint32_t num;

			cin >> num;
			ids.push_back(num);
		}
	}

	if (!limit) {
		cout << endl << "enter limit: ";
		cin >> limit;
		cout << endl;
	}

	getSimilarCharacters(ids, limit);
	
	getchar();
	cout << "press enter to exit..." << endl;
	getchar();

	return 0;
}
