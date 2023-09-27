#pragma once

#include "BlockStore/block.h"


using namespace BlockStore;


template<class T = void>
class Block;


template<>
class Block<> {
private:
	friend class RootFrame;

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
protected:
	static void LoadChild(Block& child, block_ref ref) { child.ref = ref; child.Load(); child.ResetModified(); }
	static block_ref GetChildRef(Block& child) { if (child.ref.empty()) { child.DoSave(); } return child.ref; }
protected:
	virtual void Load() {}
	virtual void Save() {}
private:
	static void SaveAll();
};


template<class T>
class Block : public Block<> {
protected:
	using value_type = T;
private:
	block<value_type>& GetRef() { return static_cast<block<value_type>&>(ref); }
private:
	virtual void Load() override final { Set(GetRef().read()); }
	virtual void Save() override final { GetRef().write(Get()); }
protected:
	virtual void Set(const value_type& value) {}
	virtual value_type Get() { return {}; }
};
