#pragma once

#include "BlockStore/block.h"


using namespace BlockStore;


template<class T = void>
class Block;


template<>
class Block<> {
public:
	Block() { DataModified(); }
	~Block() { ResetModified(); }

protected:
	block<> ref;
private:
	bool modified = false;
	bool save_error = false;
public:
	bool IsModified() const { return modified; }
	bool HasSaveError() const { return save_error; }
protected:
	void DataModified();
	void ResetModified();
	void DoSave();
public:
	static void LoadBlock(Block& block, block_ref ref) { block.ref = ref; block.Load(); block.ResetModified(); }
	static block_ref GetBlockRef(Block& block) { if (block.ref.empty()) { block.DoSave(); } return block.ref; }
protected:
	virtual void Load() {}
	virtual void Save() {}
public:
	static void SaveAll();
};


template<class T>
class Block : public Block<> {
protected:
	using value_type = T;
private:
	block<value_type>& Ref() { return static_cast<block<value_type>&>(ref); }
private:
	virtual void Load() override final { Set(Ref().read()); }
	virtual void Save() override final { Ref().write(Get()); }
protected:
	virtual void Set(const value_type& value) {}
	virtual value_type Get() { return {}; }
};
