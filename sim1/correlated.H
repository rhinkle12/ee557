/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2012 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
class CORRELATED
{
  public:
    CORRELATED();
    UINT64 References() const { return _references;};
    UINT64 Predicts() const { return _predicts;};
    UINT64 Mispredicts() const { return _references - _predicts;};
    VOID Activate()
    {
        INS_AddInstrumentFunction(Instruction, this);
    }

  private:
    enum 
    {
        ADRESSSPACE = 16, //4 bits for pc
        HISTORY = 16, // 4 bits for global history
        BUNDLESIZE = 1,
        MAXTHREADS = 100
    };

    static VOID Instruction(INS ins, VOID *v);
    static VOID CondBranch(CORRELATED *bm, INT8 * branchHistory, VOID * ip,  BOOL taken);
    inline INT8 * HistAddress(INS ins);

    INT8 _branchHistory[ADRESSSPACE * HISTORY][BUNDLESIZE];
    UINT64 _references, _predicts;
    UINT8 global_history;
};

inline CORRELATED::CORRELATED()
{
    _references = 0;
    _predicts = 0;
    global_history = 0;
}

inline VOID CORRELATED::Instruction(INS ins, VOID *v)
{
    CORRELATED *bm = static_cast<CORRELATED*>(v);
    if (INS_IsBranchOrCall(ins) && INS_HasFallThrough(ins))
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, 
                           (AFUNPTR)CondBranch, IARG_PTR, (void *)bm, IARG_PTR, (void *)bm->HistAddress(ins), IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_END);
        }
}

inline INT8 * CORRELATED::HistAddress(INS ins)
{
    ADDRINT ip = (INS_Address(ins) & 0xf)  | ((0xf & global_history)<<4); 
    // locates correct position in branch prediction buffer top 4 bits global history bottom 4 bits PC
    return &(_branchHistory[ip][0]);
}

inline VOID CORRELATED::CondBranch(CORRELATED *bm, INT8 * branchHistory, 
VOID * ip, BOOL taken)
{
    INT8 history;

    bm->_references++; // inc number of branches taken
  
    history = *branchHistory & 0x3; // mask hitory only bottom two bits
    bm->_predicts += (taken && history >= 2); // correctly predits taken
    bm->_predicts += (!taken && history <= 1); // correclty predicts not taken

    INT8 delta = 0;
    if (taken && history < 3) // move twoards more strongly taken
    {
        delta = 1;
    }
    if (!taken && history > 0) // move twords more strongly not taken
    {
        delta = -1;
    }
    
    if(taken)
    {
        bm->global_history = (bm->global_history<<1) | 0x1;; // modify global history if taken
    }
    else
    {
        bm->global_history = bm->global_history<<1; // modify global history if not taken
    }
     



  
    *branchHistory += delta; // update buffer
}