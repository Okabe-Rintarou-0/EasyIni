#include "../ini_parser.h"
#include <SimpleIni.h>

#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstdlib>

// ============================================================
// Model — 20 fields
// ============================================================

struct BenchModel {
    std::string key00; std::string key01; std::string key02; std::string key03; std::string key04;
    std::string key05; std::string key06; std::string key07; std::string key08; std::string key09;
    std::string key10; std::string key11; std::string key12; std::string key13; std::string key14;
    std::string key15; std::string key16; std::string key17; std::string key18; std::string key19;
};

// ============================================================
// Helper: generate large INI file (1000 lines, 50× each key)
// ============================================================

static const char* bench_path = "benchmark/large.ini";

static void ensure_large_ini() {
    std::ifstream f(bench_path);
    if (f.is_open()) return;

    std::ofstream out(bench_path);
    out << "[BenchModel]\n";
    for (int i = 0; i < 1000; i++) {
        out << "key" << (i % 20) << " = value_" << i << "\n";
    }
}

// ============================================================
// Simple benchmark helper
// ============================================================

using Clock = std::chrono::steady_clock;

struct BenchResult {
    const char* name;
    double time_us;   // median across iterations
};

static volatile int g_sink = 0;

static BenchResult run_bench(const char* name, int iterations, auto fn) {
    std::vector<double> samples;
    samples.reserve(iterations);

    for (int i = 0; i < iterations; i++) {
        auto start = Clock::now();
        fn();
        auto end = Clock::now();
        double ns = std::chrono::duration<double, std::nano>(end - start).count();
        samples.push_back(ns);
    }

    std::sort(samples.begin(), samples.end());
    double median = samples[iterations / 2];

    std::cout << name << "," << iterations << "," << median << ",ns" << std::endl;

    return {name, median};
}

// ============================================================
// Main
// ============================================================

int main() {
    ensure_large_ini();
    constexpr int N = 100;

    std::cout << "name,iterations,real_time,time_unit" << std::endl;

    // 1) Reflection parser
    run_bench("BM_ReflectionParse", N, [] {
        BenchModel m;
        IniParser::parse(bench_path, m);
        g_sink = m.key00.size();
    });
    // 2) SimpleIni LoadFile + bind
    run_bench("BM_SimpleIniLoadAndBind", N, [] {
        CSimpleIniA ini;
        ini.SetUnicode();
        SI_Error rc = ini.LoadFile(bench_path);
        if (rc < 0) std::abort();
        BenchModel m;
        m.key00 = ini.GetValue("BenchModel", "key00", "");
        m.key01 = ini.GetValue("BenchModel", "key01", "");
        m.key02 = ini.GetValue("BenchModel", "key02", "");
        m.key03 = ini.GetValue("BenchModel", "key03", "");
        m.key04 = ini.GetValue("BenchModel", "key04", "");
        m.key05 = ini.GetValue("BenchModel", "key05", "");
        m.key06 = ini.GetValue("BenchModel", "key06", "");
        m.key07 = ini.GetValue("BenchModel", "key07", "");
        m.key08 = ini.GetValue("BenchModel", "key08", "");
        m.key09 = ini.GetValue("BenchModel", "key09", "");
        m.key10 = ini.GetValue("BenchModel", "key10", "");
        m.key11 = ini.GetValue("BenchModel", "key11", "");
        m.key12 = ini.GetValue("BenchModel", "key12", "");
        m.key13 = ini.GetValue("BenchModel", "key13", "");
        m.key14 = ini.GetValue("BenchModel", "key14", "");
        m.key15 = ini.GetValue("BenchModel", "key15", "");
        m.key16 = ini.GetValue("BenchModel", "key16", "");
        m.key17 = ini.GetValue("BenchModel", "key17", "");
        m.key18 = ini.GetValue("BenchModel", "key18", "");
        m.key19 = ini.GetValue("BenchModel", "key19", "");
        g_sink = m.key00.size();
    });
    // 3) SimpleIni bind only
    {
        CSimpleIniA ini;
        ini.SetUnicode();
        SI_Error rc = ini.LoadFile(bench_path);
        if (rc < 0) std::abort();
        run_bench("BM_SimpleIniBindOnly", N, [&] {
            BenchModel m;
            m.key00 = ini.GetValue("BenchModel", "key00", "");
            m.key01 = ini.GetValue("BenchModel", "key01", "");
            m.key02 = ini.GetValue("BenchModel", "key02", "");
            m.key03 = ini.GetValue("BenchModel", "key03", "");
            m.key04 = ini.GetValue("BenchModel", "key04", "");
            m.key05 = ini.GetValue("BenchModel", "key05", "");
            m.key06 = ini.GetValue("BenchModel", "key06", "");
            m.key07 = ini.GetValue("BenchModel", "key07", "");
            m.key08 = ini.GetValue("BenchModel", "key08", "");
            m.key09 = ini.GetValue("BenchModel", "key09", "");
            m.key10 = ini.GetValue("BenchModel", "key10", "");
            m.key11 = ini.GetValue("BenchModel", "key11", "");
            m.key12 = ini.GetValue("BenchModel", "key12", "");
            m.key13 = ini.GetValue("BenchModel", "key13", "");
            m.key14 = ini.GetValue("BenchModel", "key14", "");
            m.key15 = ini.GetValue("BenchModel", "key15", "");
            m.key16 = ini.GetValue("BenchModel", "key16", "");
            m.key17 = ini.GetValue("BenchModel", "key17", "");
            m.key18 = ini.GetValue("BenchModel", "key18", "");
            m.key19 = ini.GetValue("BenchModel", "key19", "");
            g_sink = m.key00.size();
        });
    }

    return 0;
}
