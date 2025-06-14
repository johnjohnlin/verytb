// Direct include
#include "Vpicorv32_axi.h"
// C system headers
// C++ standard library headers
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
// Other libraries' .h files.
#include "immintrin.h"
#include "verilated_fst_c.h"
// Your project's .h files.

using namespace std;

struct ar_s {
	uint8_t prot;
	uint32_t addr;
};
using aw_s = ar_s;

struct r_s {
	uint32_t data;
};

struct w_s {
	uint32_t data;
	uint8_t strb;
};

struct b_s {
};

template <typename T>
void OptionalToQueue(optional<T>& opt, deque<T>& dq, unsigned max_size) {
	if (opt.has_value() and dq.size() < max_size) {
		dq.push_back(move(*opt));
		opt.reset();
	}
}

template <typename T>
void QueueToOptional(deque<T>& dq, optional<T>& opt) {
	if (!opt.has_value() && !dq.empty()) {
		opt = move(dq.front());
		dq.pop_front();
	}
}

void BitMaskToByteMask(uint32_t& byte_mask, uint8_t bit_mask) {
	byte_mask = _pdep_u32(bit_mask, 0x1010'1010u);
	byte_mask -= byte_mask >> 4;
	byte_mask |= byte_mask << 4;
}

void BitMaskToByteMask(uint64_t& byte_mask, uint8_t bit_mask) {
	byte_mask = _pdep_u64(bit_mask, 0x1010'1010'1010'1010llu);
	byte_mask -= byte_mask >> 4;
	byte_mask |= byte_mask << 4;
}

struct Vpicorv32Wrap {
	optional<ar_s>* ar;
	optional<aw_s>* aw;
	optional<r_s>* r;
	optional<w_s>* w;
	optional<b_s>* b;
	uint32_t *irq;

private:
	VerilatedContext ctx;
	VerilatedFstC tfp;
	Vpicorv32_axi dut;
	unsigned timestamp = 0;

public:

	Vpicorv32Wrap() {
		ctx.traceEverOn(true);
		dut.trace(&tfp, 99);
		tfp.open("dump.fst");

		dut.resetn = 1;
		for (unsigned i = 0; i < 5; ++i) { Eval(); }
		dut.resetn = 0;
		for (unsigned i = 0; i < 3; ++i) { Eval(); }
		dut.resetn = 1;
		for (unsigned i = 0; i < 5; ++i) { Eval(); }
	}

	~Vpicorv32Wrap() {
		tfp.close();
	}

	void Eval() {
		tfp.dump(timestamp++);
		dut.clk = 0;
		dut.eval();
		tfp.dump(timestamp++);
		dut.clk = 1;
		dut.eval();
	}

	void Pre() {
		// better to use RAII to manage

		// Only Valid/Ready & Signal input and signal out are put here
		dut.mem_axi_rvalid = r->has_value();
		if (dut.mem_axi_rvalid) {
			dut.mem_axi_rdata = r->value().data;
		}
		dut.mem_axi_bvalid = b->has_value();
		dut.irq = *irq;

		dut.eval();

		if (dut.mem_axi_rvalid and dut.mem_axi_rready) {
			r->reset();
		}
		if (dut.mem_axi_bvalid and dut.mem_axi_bready) {
			b->reset();
		}
	}

	void Post() {
		// better to use RAII to manage
		bool arready = not ar->has_value();
		bool awready = not aw->has_value();
		bool wready = not w->has_value();
		bool arvalid = dut.mem_axi_arvalid;
		bool awvalid = dut.mem_axi_awvalid;
		bool wvalid = dut.mem_axi_wvalid;
		dut.mem_axi_arready = arready;
		dut.mem_axi_awready = awready;
		dut.mem_axi_wready = wready;
		Eval();
		if (arvalid and arready) {
			*ar = {dut.mem_axi_arprot, dut.mem_axi_araddr};
		}
		if (awvalid and awready) {
			*aw = {dut.mem_axi_awprot, dut.mem_axi_awaddr};
		}
		if (wvalid and wready) {
			*w = {dut.mem_axi_wdata, dut.mem_axi_wstrb};
		}
	}
};

struct AxiMemory {
	optional<ar_s>* ar;
	optional<aw_s>* aw;
	optional<r_s>* r;
	optional<w_s>* w;
	optional<b_s>* b;
	vector<uint32_t> memory_space;
	bool simulation_passed = false;

private:
	deque<ar_s> ar_q;
	deque<aw_s> aw_q;
	deque<r_s> r_q;
	deque<w_s> w_q;
	deque<b_s> b_q;

public:

	AxiMemory() {
	}

	void HandleInput_() {
		OptionalToQueue(*ar, ar_q, 128);
		OptionalToQueue(*aw, aw_q, 128);
		OptionalToQueue(*w, w_q, 128);
	}

	void Exec_() {
		if (not ar_q.empty() and r_q.size() < 16) {
			auto& ar_data = ar_q.front();

			r_q.push_back({memory_space[ar_data.addr / 4]});

			ar_q.pop_front();
		}
		if (not aw_q.empty() and not w_q.empty() and b_q.size() < 16) {
			auto& aw_data = aw_q.front();
			auto& w_data = w_q.front();
			if (aw_data.addr == 0x10000000u) {
				const char c = char(w_data.data);
				cout << c;
			} else if (aw_data.addr == 0x20000000u) {
				cout << "Simulation passed" << endl;
				simulation_passed = true;
			} else {
				uint32_t& dst = memory_space[aw_data.addr / 4];
				uint32_t src = w_data.data;
				uint32_t mask;
				BitMaskToByteMask(mask, w_data.strb);
				dst = (src & mask) | (dst & ~mask);
			}


			b_q.emplace_back();
			aw_q.pop_front();
			w_q.pop_front();
		}
	}

	void HandleInputAndExec() {
		HandleInput_();
		Exec_();
	}

	void HandleOutput() {
		QueueToOptional(r_q, *r);
		QueueToOptional(b_q, *b);
	}
};

struct IrqDriver {
	uint32_t *irq;
	void Main() {
		const bool irq4 = (counter & 0xfffu) == 0xfffu;
		const bool irq5 = counter == 0xffffu;
		*irq = (unsigned(irq4) << 4) | (unsigned(irq5) << 5);
		++counter;
	}
private:
	uint16_t counter = 0;
};

vector<uint32_t> ReadBin(const string& filename) {
	vector<uint32_t> data;
	ifstream file(filename, ios::binary | ios::ate);
	if (not file) {
		cerr << "Failed to open file: " << filename << endl;
		abort();
	}

	streamsize size = file.tellg();
	if (size % sizeof(uint32_t) != 0) {
		cerr << "File size is not aligned to 4 bytes." << endl;
		abort();
	}

	file.seekg(0, ios::beg);
	data.resize(size / sizeof(uint32_t));

	if (not file.read(reinterpret_cast<char *>(data.data()), size)) {
		cerr << "Failed to read data from file." << endl;
		abort();
	}

	return data;
}

int main() {
	// Channe
	optional<ar_s> ar;
	optional<aw_s> aw;
	optional<r_s> r;
	optional<w_s> w;
	optional<b_s> b;
	uint32_t irq;

	// Module
	Vpicorv32Wrap dut;
	AxiMemory axi;
	IrqDriver irq_driver;
	axi.memory_space = ReadBin("firmware.bin");
	axi.memory_space.resize(1<<16); // 1MB

	// Connect
	dut.ar = &ar;
	dut.aw = &aw;
	dut.r = &r;
	dut.w = &w;
	dut.b = &b;
	dut.irq = &irq;

	axi.ar = &ar;
	axi.aw = &aw;
	axi.r = &r;
	axi.w = &w;
	axi.b = &b;

	irq_driver.irq = &irq;

	// main simulation kernel
	for (unsigned i = 0; i < 1000000 and not axi.simulation_passed; ++i) {
		irq_driver.Main();
		axi.HandleInputAndExec();
		dut.Pre();

		axi.HandleOutput();
		dut.Post();
	}
	return 0;
}
