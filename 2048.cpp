/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++0x -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */
#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>
#include <list>
#include <array>
#include <limits>
#include <numeric>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>

/**
 * array-based board for 2048
 *
 * index (2-d form):
 * [0][0] [0][1] [0][2] [0][3]
 * [1][0] [1][1] [1][2] [1][3]
 * [2][0] [2][1] [2][2] [2][3]
 * [3][0] [3][1] [3][2] [3][3]
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {

public:
	board() : tile() {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	std::array<int, 4>& operator [](const int& i) { return tile[i]; }
	const std::array<int, 4>& operator [](const int& i) const { return tile[i]; }
	int& operator ()(const int& i) { return tile[i / 4][i % 4]; }
	const int& operator ()(const int& i) const { return tile[i / 4][i % 4]; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
	/**
	 * apply an action to the board
	 * return the reward gained by the action, or -1 if the action is illegal
	 */
	int move(const int& opcode) {
		switch (opcode) {
		case 0: return move_up();
		case 1: return move_right();
		case 2: return move_down();
		case 3: return move_left();
		default: return -1;
		}
	}

	int move_left() {
		board prev = *this;
		int score = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
				if (tile == 0) continue;
				row[c] = 0;
				if (hold) {
					if (tile == hold) {
						row[top++] = ++tile;
						score += (1 << tile);
						hold = 0;
					} else {
						row[top++] = hold;
						hold = tile;
					}
				} else {
					hold = tile;
				}
			}
			if (hold) tile[r][top] = hold;
		}
		return (*this != prev) ? score : -1;
	}
	int move_right() {
		reflect_horizontal();
		int score = move_left();
		reflect_horizontal();
		return score;
		return score;
	}
	int move_up() {
		rotate_right();
		int score = move_right();
		rotate_left();
		return score;
	}
	int move_down() {
		rotate_right();
		int score = move_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(const int& r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const board& b) {
		char buff[32];
		out << "+------------------------+" << std::endl;
		for (int r = 0; r < 4; r++) {
			snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
				(1 << b[r][0]) & -2u, // use -2u (0xff...fe) to remove the unnecessary 1 for (1 << 0)
				(1 << b[r][1]) & -2u,
				(1 << b[r][2]) & -2u,
				(1 << b[r][3]) & -2u);
			out << buff << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

private:
    std::array<std::array<int, 4>, 4> tile;
};

class action {
public:
	action(const action& act) = default;
	action(const int& op = -1) : opcode(op) {}
	operator int() const { return opcode; }

	int apply(board& b) const {
		if ((0b11 & opcode) == (opcode)) {
			return b.move(opcode);
		} else if (b(opcode & 0x0f) == 0) {
			b(opcode & 0x0f) = (opcode >> 4);
			return 0;
		}
		return -1;
	}

	std::string name() const {
		if ((0b11 & opcode) == (opcode)) {
			std::string opname[] = { "up", "right", "down", "left" };
			return "slide " + opname[opcode];
		} else {
			return "place " + std::to_string(opcode >> 4) + "-index at position " + std::to_string(opcode & 0x0f);
		}
		return "null";
	}

	static action move(const int& oper) {
		return action(oper);
	}
	static action place(const int& tile, const int& pos) {
		return action((tile << 4) | (pos));
	}

private:
	const int opcode;
};

class agent {
public:
	agent() {}
	virtual ~agent() {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }
	virtual std::string name() const { return "null"; }
};

class player : public agent {
public:
	player() {}

	/**
	 * select an action by immediately reward
	 */
	virtual action take_action(const board& before) {
		int code = -1;
		int reward = -1;
		for (int i = 0; i < 4; i++) {
			board b = before;
			int r = b.move(i);
			if (r > reward) {
				reward = r;
				code = i;
			}
		}
		return action::move(code);
	}
	virtual std::string name() const { return "player"; }

private:
};

class random : public agent {
public:
	random(const uint64_t& seed = 0) : engine(seed) {}

	/**
	 * add a new random tile on board, or do nothing if the board is full
	 * 2-tile: 90%
	 * 4-tile: 10%
	 */
	virtual action take_action(const board& after) {
		int space[16], num = 0;
		for (int i = 0; i < 16; i++)
			if (after(i) == 0) {
				space[num++] = i;
			}
		if (num) {
			std::uniform_int_distribution<int> popup(0, 9);
			std::uniform_int_distribution<int> empty(0, num - 1);

			int tile = popup(engine) ? 1 : 2;
			int pos = space[empty(engine)];
			return action::place(tile, pos);
		} else {
			return action();
		}
	}
	virtual std::string name() const { return "random"; }

private:
	std::default_random_engine engine;
};

class statistic {
public:
	statistic(const size_t& total, const size_t& unit = 0) : total(total), unit(unit ? unit : total) {}

public:
	/**
	 * show the statistic of last unit games
	 *
	 * the format would be
	 * 1000   avg = 273901, max = 382324, ops = 241563
	 *        512     100%   (0.3%)
	 *        1024    99.7%  (0.2%)
	 *        2048    99.5%  (1.1%)
	 *        4096    98.4%  (4.7%)
	 *        8192    93.7%  (22.4%)
	 *        16384   71.3%  (71.3%)
	 *
	 * where (assume that unit = 1000)
	 *  '1000': current index (n)
	 *  'avg = 273901': the average score of saved games is 273901
	 *  'max = 382324': the maximum score of saved games is 382324
	 *  'ops = 241563': the average speed of saved games is 241563
	 *  '93.7%': 93.7% (937 games) reached 8192-tiles in saved games (a.k.a. win rate of 8192-tile)
	 *  '22.4%': 22.4% (224 games) terminated with 8192-tiles (the largest) in saved games
	 */
	void show() {
		int unit = std::min(data.size(), this->unit);
		size_t sum = 0, max = 0, opc = 0, stat[16] = { 0 };
		auto it = data.end();
		for (int i = 0; i < unit; i++) {
			auto& path = *(--it);
			board game;
			opc += path.size();
			size_t score = 0;
			for (action& move : path)
				score += move.apply(game);
			sum += score;
			max = std::max(score, max);
			int tile = 0;
			for (int i = 0; i < 16; i++)
				tile = std::max(tile, game(i));
			stat[tile]++;
		}
		float avg = float(sum) / unit;
		float coef = 100.0 / unit;
		float ops = opc * 1000.0 / (data.back().tock_time() - it->tick_time());
		std::cout << data.size() << "\t";
		std::cout << "avg = " << int(avg) << ", ";
		std::cout << "max = " << int(max) << ", ";
		std::cout << "ops = " << int(ops) << std::endl;
		for (int t = 0, c = 0; c < unit; c += stat[t++]) {
			if (stat[t] == 0) continue;
			int accu = std::accumulate(stat + t, stat + 16, 0);
			std::cout << "\t" << ((1 << t) & -2u) << "\t" << (accu * coef) << "%";
			std::cout << "\t(" << (stat[t] * coef) << "%)" << std::endl;
		}
		std::cout << std::endl;
	}

	void summary() {
		auto unit_temp = unit;
		unit = data.size();
		show();
		unit = unit_temp;
	}

	void open_episode() {
		data.emplace_back();
		data.back().tick();
	}

	void close_episode(const bool& disp = true) {
		data.back().tock();
		if (disp && data.size() % unit == 0) show();
	}
	
	bool is_finished() const {
		return data.size() >= total;
	}

	void save_action(const action& move) {
		data.back().push_back(move);
	}

	agent& take_turns(agent& play, agent& evil) {
		// 1:play 0:evil
		return (std::max(data.back().size() + 1, 2ull) % 2) ? play : evil;
	}

	friend std::ostream& operator <<(std::ostream& out, const statistic& stat) {
		auto total = stat.total;
		auto unit = stat.unit;
		auto size = stat.data.size();
		out.write(reinterpret_cast<char*>(&total), sizeof(total));
		out.write(reinterpret_cast<char*>(&unit), sizeof(unit));
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (const record& rec : stat.data) out << rec;
		return out;
	}
	friend std::istream& operator >>(std::istream& in, statistic& stat) {
		auto total = stat.total;
		auto unit = stat.unit;
		auto size = stat.data.size();
		in.read(reinterpret_cast<char*>(&total), sizeof(total));
		in.read(reinterpret_cast<char*>(&unit), sizeof(unit));
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		stat.total = total;
		stat.unit = unit;
		for (size_t i = 0; i < size; i++) {
			stat.data.emplace_back();
			in >> stat.data.back();
		}
		return in;
	}

private:
	class record : public std::vector<action> {
	public:
		record() { reserve(32768); }
		void tick() { time[0] = milli(); }
		void tock() { time[1] = milli(); }
		uint64_t tick_time() const { return time[0]; }
		uint64_t tock_time() const { return time[1]; }

		friend std::ostream& operator <<(std::ostream& out, const record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			out.write(reinterpret_cast<char*>(&size), sizeof(size));
			for (const action& act : rec) {
				out.write(reinterpret_cast<const char*>(&act), sizeof(act));
			}
			out.write(reinterpret_cast<const char*>(time), sizeof(time[0]) * 2);
			return out;
		}
		friend std::istream& operator >>(std::istream& in, record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			in.read(reinterpret_cast<char*>(&size), sizeof(size));
			rec.reserve(size);
			for (auto i = 0ull; i < size; i++) {
				rec.emplace_back();
				in.read(reinterpret_cast<char*>(&rec.back()), sizeof(rec.back()));
			}
			in.read(reinterpret_cast<char*>(time), sizeof(time[0]) * 2);
			return in;
		}

	private:
		uint64_t milli() const {
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}
		uint64_t time[2];
	};

	size_t total;
	size_t unit;
	std::list<record> data;
};

int main(int argc, const char* argv[]) {
	std::cout << "2048-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, unit = 0;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--unit=") == 0) {
			unit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--load=") == 0) {
			load = para.substr(para.find("=") + 1);
		} else if (para.find("--save=") == 0) {
			save = para.substr(para.find("=") + 1);
		} else if (para.find("--summary") == 0) {
			summary = true;
		}
	}

	player play;
	random evil;

	statistic stat(total, unit);

	if (load.size()) {
		std::ifstream in;
		in.open(load.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) return -1;
		in >> stat;
		in.close();
	}

	while (!stat.is_finished()) {
		stat.open_episode();
		board game;
		while (true) {
			agent& who = stat.take_turns(play, evil);
			action move = who.take_action(game);
			int reward = move.apply(game);
			if (reward == -1) break;
			stat.save_action(move);
			if (who.check_for_win(game)) break;
		}
		stat.close_episode();
	}

	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out;
		out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return -1;
		out << stat;
		out.flush();
		out.close();
	}

	return 0;
}
