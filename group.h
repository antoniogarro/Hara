/***************************************************************************************
* Copyright (c) 2014, Antonio Garro.                                                   *
* All rights reserved                                                                  *
*                                                                                      *
* Redistribution and use in source and binary forms, with or without modification, are *
* permitted provided that the following conditions are met:                            *
*                                                                                      *
* 1. Redistributions of source code must retain the above copyright notice, this list  *
* of conditions and the following disclaimer.                                          *
*                                                                                      *
* 2. Redistributions in binary form must reproduce the above copyright notice, this    *
* list of conditions and the following disclaimer in the documentation and/or other    *
* materials provided with the distribution.                                            *
*                                                                                      *
* 3. Neither the name of the copyright holder nor the names of its contributors may be *
* used to endorse or promote products derived from this software without specific      *
* prior written permission.                                                            *
*                                                                                      * 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"          *
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE            *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE           *
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE            *
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL    *
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR           *
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER           *
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     *
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF        *
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                    *
***************************************************************************************/
#ifndef GROUPH
#define GROUPH

#include "size.h"
#include <iostream>

class PList{
public:
	virtual void add(int p) = 0;
	virtual void remove(int p) = 0;
	virtual int operator[](int i)const = 0;
	virtual int length() const = 0;

};

class Group{
private:
	bool color;		//color = 0 for Black, 1 for White.
	int group_stones[MAXSIZE2];
	int nstones;
	int group_liberties[MAXSIZE2*2/3];
	int nliberties;

public:
	Group();
	void set_up(int, bool, const PList&);
	void clear();
	void attach_group(Group*);
	
	bool get_color() const{ return color; }
	
	int get_nstones() const{ return nstones; }
	int get_stone(int i) const{ return group_stones[i]; }
	const int *get_stones() const { return group_stones; }
	
	int get_nliberties() const{ return nliberties; }
	int get_liberty(int i) const{ return group_liberties[i]; }
	const int *get_liberties() const{ return group_liberties; }
	
	class StoneIterator{
	protected:
		const int *it;
	public:
		StoneIterator(){ it = 0; }
		StoneIterator(const Group *gr)
		{
			it = gr->get_stones();
		}
		void operator++()
		{
			++it;
		}
		int operator*() const
		{
			return *it;
		}
		operator bool() const
		{
			return *it != 0;
		}
	};
	
	class LibertyIterator : public StoneIterator{
	public:
		LibertyIterator(const Group *gr)
		{
			it = gr->get_liberties();
		}
	};
	
	bool has_one_liberty() const
	{
		return nliberties == 1;
	}
	
	bool has_two_liberties() const
	{
		return nliberties == 2;
	}
	int add_liberties(int);
	int erase_liberties(int);
	
	void print_group() const;
};

template<const int S> class GroupSet{
protected:
	Group *groups[S];
	int multiplicity[S];
	int len;

public:
	GroupSet()
	{
		len = 0;
		groups[0] = 0;
	}

	bool add(Group *gr)
	{
		if(gr == 0) return false;
		for(int i = 0; i < len; i++){
			if(groups[i] == gr){
				multiplicity[i]++;
				return  false;
			}
		}
		multiplicity[len] = 1;
		groups[len++] = gr;
		return true;
	}

	Group *operator[](int i) const{ return groups[i]; }
	int get_multiplicity(int i) const
	{
		#ifdef DEBUG_INFO
		if(i >= len){
			std::cerr << "WARNING: No multiplicity(i)\n";
			return 0;
		}
		#endif
		return multiplicity[i];
	}
	int length() const{ return len; }
};

template<int S> class PointList : public PList{
protected:
	int points[S];
	int len;

public:
	PointList()
	{
		len = 0;
		points[0] = 0;
	}
	void clear()
	{
		len = 0;
		points[0] = 0;
	}
	virtual void add(int p)
	{
		if(len == S){
		#ifdef DEBUG_INFO
			std::cerr << "long list\n";
		#endif		
			return;
		}
		points[len] = p;
		points[++len] = 0;
	}
	void remove(int p)
	{
		for(int j = 0; j < len; j++){
			if(points[j] == p){
				points[j] = points[--len];
				points[len] = 0;
				break;
			}
		}
	}

	int operator[](int i)const{ return points[i]; }
	int length() const{ return len; }

};

template<int S> class PointSet : public PointList<S>{
public:
	void add(int p)
	{
		if(PointList<S>::len == S){
		#ifdef DEBUG_INFO
			std::cerr << "long set\n";
		#endif
			return;
		}
		for(int i = 0; i < PointList<S>::len; i++){
			if(PointList<S>::points[i] == p) return;
		}
		PointList<S>::points[PointList<S>::len] = p;
		PointList<S>::points[++PointList<S>::len] = 0;
	}
};
#endif
