void BACKEND_NAME::Emit(Binop<i32>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(Binop<u32>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(Binop<i64>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(Binop<u64>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(Binop<float>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(Binop<double>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(BinopImm<i32>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(BinopImm<u32>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(BinopImm<i64>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(BinopImm<u64>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(BinopImm<float>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(BinopImm<double>& op) { EmitBinopImm(op); }
void BACKEND_NAME::Emit(Shift<i32>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(Shift<u32>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(Shift<i64>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(Shift<u64>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(ShiftImm<i32>& op) { EmitShiftImm(op); }
void BACKEND_NAME::Emit(ShiftImm<u32>& op) { EmitShiftImm(op); }
void BACKEND_NAME::Emit(ShiftImm<i64>& op) { EmitShiftImm(op); }
void BACKEND_NAME::Emit(ShiftImm<u64>& op) { EmitShiftImm(op); }
void BACKEND_NAME::Emit(Compare<i32>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<u32>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<i64>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<u64>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<float>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<double>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(Compare<Pointer>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(CompareImm<i32>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<u32>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<i64>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<u64>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<float>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<double>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(CompareImm<Pointer>& op) { EmitCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompare<i32>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<u32>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<i64>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<u64>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<float>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<double>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompare<Pointer>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(BranchCompareImm<i32>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<u32>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<i64>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<u64>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<float>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<double>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(BranchCompareImm<Pointer>& op) { EmitBranchCompareImm(op); }
void BACKEND_NAME::Emit(AddToPointer<i32>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(AddToPointer<u32>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(AddToPointer<i64>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(AddToPointer<u64>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(Unop<i32>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Unop<u32>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Unop<i64>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Unop<u64>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Unop<float>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Unop<double>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(Imm<i32>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(Imm<u32>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(Imm<i64>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(Imm<u64>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(Imm<float>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(Imm<double>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(LoadLocal<i8>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<u8>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<i16>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<u16>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<i32>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<u32>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<i64>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<u64>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<float>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<double>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<Struct>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(LoadLocal<Pointer>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<i8>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<u8>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<i16>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<u16>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<i32>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<u32>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<i64>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<u64>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<float>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<double>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<Struct>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(StoreLocal<Pointer>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(LoadGlobal<i8>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<u8>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<i16>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<u16>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<i32>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<u32>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<i64>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<u64>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<float>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<double>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<Struct>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(LoadGlobal<Pointer>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<i8>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<u8>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<i16>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<u16>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<i32>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<u32>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<i64>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<u64>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<float>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<double>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<Struct>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(StoreGlobal<Pointer>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(LoadFromPointer<i8>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<u8>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<i16>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<u16>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<i32>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<u32>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<i64>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<u64>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<float>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<double>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<Struct>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(LoadFromPointer<Pointer>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<i8>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<u8>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<i16>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<u16>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<i32>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<u32>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<i64>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<u64>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<float>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<double>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<Struct>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(StoreToPointer<Pointer>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(Return<i32>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<u32>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<i64>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<u64>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<float>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<double>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<Struct>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Return<Pointer>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(Call<i32>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<u32>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<i64>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<u64>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<float>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<double>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<Struct>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<Pointer>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(Call<void>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(CallLabel<i32>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<u32>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<i64>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<u64>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<float>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<double>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<Struct>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<Pointer>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(CallLabel<void>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(Cast<i8, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i8, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i16, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i32, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<i64, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u8, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u16, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u32, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<u64, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<float, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<double, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(Cast<Pointer, Pointer>& op) { EmitCast(op); }