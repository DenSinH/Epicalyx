void BACKEND_NAME::Emit(const Binop<i32>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Binop<u32>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Binop<i64>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Binop<u64>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Binop<float>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Binop<double>& op) { EmitBinop(op); }
void BACKEND_NAME::Emit(const Shift<i32>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(const Shift<u32>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(const Shift<i64>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(const Shift<u64>& op) { EmitShift(op); }
void BACKEND_NAME::Emit(const Compare<i32>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<u32>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<i64>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<u64>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<float>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<double>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const Compare<Pointer>& op) { EmitCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<i32>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<u32>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<i64>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<u64>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<float>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<double>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const BranchCompare<Pointer>& op) { EmitBranchCompare(op); }
void BACKEND_NAME::Emit(const AddToPointer<i32>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(const AddToPointer<u32>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(const AddToPointer<i64>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(const AddToPointer<u64>& op) { EmitAddToPointer(op); }
void BACKEND_NAME::Emit(const Unop<i32>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Unop<u32>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Unop<i64>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Unop<u64>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Unop<float>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Unop<double>& op) { EmitUnop(op); }
void BACKEND_NAME::Emit(const Imm<i32>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<u32>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<i64>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<u64>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<float>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<double>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const Imm<Pointer>& op) { EmitImm(op); }
void BACKEND_NAME::Emit(const LoadLocal<i8>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<u8>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<i16>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<u16>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<i32>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<u32>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<i64>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<u64>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<float>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<double>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<Struct>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const LoadLocal<Pointer>& op) { EmitLoadLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<i8>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<u8>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<i16>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<u16>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<i32>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<u32>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<i64>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<u64>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<float>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<double>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<Struct>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const StoreLocal<Pointer>& op) { EmitStoreLocal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<i8>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<u8>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<i16>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<u16>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<i32>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<u32>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<i64>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<u64>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<float>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<double>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<Struct>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const LoadGlobal<Pointer>& op) { EmitLoadGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<i8>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<u8>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<i16>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<u16>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<i32>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<u32>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<i64>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<u64>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<float>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<double>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<Struct>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const StoreGlobal<Pointer>& op) { EmitStoreGlobal(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<i8>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<u8>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<i16>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<u16>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<i32>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<u32>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<i64>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<u64>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<float>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<double>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<Struct>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const LoadFromPointer<Pointer>& op) { EmitLoadFromPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<i8>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<u8>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<i16>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<u16>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<i32>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<u32>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<i64>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<u64>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<float>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<double>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<Struct>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const StoreToPointer<Pointer>& op) { EmitStoreToPointer(op); }
void BACKEND_NAME::Emit(const Return<i32>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<u32>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<i64>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<u64>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<float>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<double>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<Struct>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<Pointer>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Return<void>& op) { EmitReturn(op); }
void BACKEND_NAME::Emit(const Call<i32>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<u32>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<i64>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<u64>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<float>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<double>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<Struct>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<Pointer>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const Call<void>& op) { EmitCall(op); }
void BACKEND_NAME::Emit(const CallLabel<i32>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<u32>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<i64>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<u64>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<float>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<double>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<Struct>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<Pointer>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const CallLabel<void>& op) { EmitCallLabel(op); }
void BACKEND_NAME::Emit(const Cast<i8, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i8, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i16, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i32, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<i64, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u8, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u16, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u32, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<u64, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<float, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<double, Pointer>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, i32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, u32>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, i64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, u64>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, float>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, double>& op) { EmitCast(op); }
void BACKEND_NAME::Emit(const Cast<Pointer, Pointer>& op) { EmitCast(op); }