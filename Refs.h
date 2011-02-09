#ifndef REFS
#define REFS

#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<cassert>
#include<string>
#include<fstream>
#include<vector>

#include "utils.h"
#include "RefSeq.h"
#include "RefSeqPolicy.h"
#include "PolyARules.h"


class Refs {
 public:
  Refs() {
    M = 0;
    seqs.clear();
  }

  ~Refs() {}

  void makeRefs(char*, RefSeqPolicy&, PolyARules&);
  void loadRefs(char*, int = 0);
  void saveRefs(char*);

  int getM() { return M; } // get number of isoforms

  //int getNS() { return M + 1; } // get number of parameters, I do not think we need it here.

  RefSeq& getRef(int sid) { return seqs[sid]; } // get a particular reference

  std::vector<RefSeq>& getRefs() { return seqs; } // may be slow, for copying the whole thing

  //lim : >=0 If mismatch > lim , return; -1 find all mismatches
  int countMismatch(const std::string& seq, int pos, const std::string& readseq, int LEN, int lim = -1) {
    int nMis = 0; // number of mismatches

    for (int i = 0; i < LEN; i++) {
      char rc = toupper(readseq[i]);
      if (seq[i + pos] == 'N' || rc == 'N' || seq[i + pos] != rc) nMis++;

      // a speed up tech
      if (lim >= 0 && nMis > lim) return nMis;
    }

    return nMis;
  }

  bool isValid(int sid, int dir, int pos, const std::string& readseq, int LEN, int C) {
    if (sid <= 0 || sid > M || dir != 0 && dir != 1 ||  pos < 0 || pos + LEN > seqs[sid].getTotLen() || LEN > (int)readseq.length()) return false;
    const std::string& seq = seqs[sid].getSeq(dir);
    return countMismatch(seq, pos, readseq, LEN, C) <= C;
  }

  // get segment from refs
  std::string getSegment(int sid, int dir, int pos, int LEN) {
    if (pos < 0 || pos + LEN > seqs[sid].getTotLen()) return "fail";

    const std::string& seq = seqs[sid].getSeq(dir);
    std::string seg = "";

    for (int i = 0; i < LEN; i++)
      seg.append(1,  seq[pos + i]);

    return seg;
  }

 private:
  int M; // # of isoforms, id starts from 1
  std::vector<RefSeq> seqs;  // reference sequences, starts from 1; 0 is for noise gene

};

//inpF in fasta format
void Refs::makeRefs(char *inpF, RefSeqPolicy& policy, PolyARules& rules) {
  //read standard fasta format here
  std::ifstream fin;
  std::string tag, line, rawseq;
  void* pt; // istream& is indeed a pointer, that's why I can use void* here

  seqs.clear();
  seqs.push_back(RefSeq()); // noise isoform

  M = 0;

  fin.open(inpF);
  if (!fin.is_open()) { fprintf(stderr, "Cannot open %s! It may not exist.\n", inpF); exit(-1); }
  pt = getline(fin, line);
  while (pt != 0 && line[0] == '>') {
    tag = line.substr(1);
    rawseq = "";
    while((pt = getline(fin, line)) && line[0] != '>') {
      rawseq += line;
    }
    assert(rawseq.size() > 0);
    ++M;
    seqs.push_back(RefSeq(tag, policy.convert(rawseq), rules.getLenAt(tag)));
  }
  fin.close();

  if (verbose) { printf("Refs.makeRefs finished!\n"); }
}

//inpF in fasta format, with sequence all in one line together
//option 0 read all, 1 do not read sequences and names
void Refs::loadRefs(char *inpF, int option) {
  std::ifstream fin;
  RefSeq seq;

  fin.open(inpF);
  if (!fin.is_open()) { fprintf(stderr, "Cannot open %s! It may not exist.\n", inpF); exit(-1); }
  seqs.clear();
  seqs.push_back(RefSeq());

  M = 0;

  bool success;
  do {
    success = seq.read(fin, option);
    if (success) {
    	seqs.push_back(seq);
        ++M;
    }
  } while (success);

  fin.close();

  assert(M + 1 == (int)seqs.size());

  if (verbose) { printf("Refs.loadRefs finished!\n"); }
}

void Refs::saveRefs(char* outF) {
  std::ofstream fout;

  fout.open(outF);
  for (int i = 1; i <= M; i++) {
    seqs[i].write(fout);
  }
  fout.close();

  if (verbose) { printf("Refs.saveRefs finished!\n"); }
}

#endif
