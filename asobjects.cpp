/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#include <list>
#include <algorithm>
//#include <curl/curl.h>
//#include <libxml/parser.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#define _USE_MATH_DEFINES
#include <cmath>

#include "abc.h"
#include "asobjects.h"
#include "flashevents.h"
#include "swf.h"
#include "compat.h"
#include "class.h"

using namespace std;
using namespace lightspark;

extern TLSDATA SystemState* sys;

REGISTER_CLASS_NAME(Array);
REGISTER_CLASS_NAME2(ASQName,"QName");
REGISTER_CLASS_NAME(Namespace);
REGISTER_CLASS_NAME(IInterface);
REGISTER_CLASS_NAME(Date);
REGISTER_CLASS_NAME(RegExp);

Array::Array()
{
	type=T_ARRAY;
	//constructor=new Function(_constructor);
	//ASObject::setVariableByQName("Call","",new Function(ASArray::Array));
	//ASObject::setVariableByQName("toString","",new Function(ASObject::_toString));
}

void Array::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

ASFUNCTIONBODY(Array,_constructor)
{
	Array* th=static_cast<Array*>(obj->implementation);
	obj->setGetterByQName("length","",new Function(_getLength));
	obj->ASObject::setVariableByQName("pop","",new Function(_pop));
	obj->ASObject::setVariableByQName("shift","",new Function(shift));
	obj->ASObject::setVariableByQName("unshift","",new Function(unshift));
	obj->ASObject::setVariableByQName("join",AS3,new Function(join));
	obj->ASObject::setVariableByQName("push",AS3,new Function(_push));
	obj->ASObject::setVariableByQName("sort",AS3,new Function(_sort));
	obj->ASObject::setVariableByQName("concat",AS3,new Function(_concat));

	if(args && args->size()!=0)
	{
		LOG(LOG_CALLS,"Called Array constructor");
		th->resize(args->size());
		for(int i=0;i<args->size();i++)
		{
			th->set(i,args->at(i));
			args->at(i)->incRef();
		}
	}
	return NULL;
}

ASFUNCTIONBODY(Array,_getLength)
{
	Array* th=static_cast<Array*>(obj->implementation);
	return abstract_i(th->data.size());
}

ASFUNCTIONBODY(Array,shift)
{
	Array* th=static_cast<Array*>(obj->implementation);
	if(th->data.empty())
		return new Undefined;
	ASObject* ret;
	if(th->data[0].type==STACK_OBJECT)
		ret=th->data[0].data;
	else
		abort();
	th->data.erase(th->data.begin());
	return ret;
}

ASFUNCTIONBODY(Array,join)
{
		Array* th=static_cast<Array*>(obj->implementation);
		ASObject* del=args->at(0);
		string ret;
		for(int i=0;i<th->size();i++)
		{
			ret+=th->at(i)->toString().raw_buf();
			if(i!=th->size()-1)
				ret+=del->toString().raw_buf();
		}
		return new ASString(ret);
}

ASFUNCTIONBODY(Array,_concat)
{
	Array* th=static_cast<Array*>(obj->implementation);
	Array* ret=Class<Array>::getInstanceS(true);
	ret->data=th->data;
	if(args->size()>=1 && args->at(0)->getObjectType()==T_ARRAY)
	{
		assert(args->size()==1);
		Array* tmp=Class<Array>::cast(args->at(0)->implementation);
		ret->data.insert(ret->data.end(),tmp->data.begin(),tmp->data.end());
	}
	else
	{
		//Insert the arguments in the array
		ret->data.insert(th->data.end(),args->data.begin(),args->data.end());
		ret->data.reserve(ret->data.size()+args->size());
		for(int i=0;i<args->size();i++)
			ret->push(args->at(i));
	}

	//All the elements in the new array should be increffed, as args will be deleted and
	//this array could die too
	for(int i=0;i<ret->data.size();i++)
	{
		
		if(ret->data[i].type==STACK_OBJECT)
			ret->data[i].data->incRef();
	}
	
	return ret->obj;
}

ASFUNCTIONBODY(Array,_pop)
{
	Array* th=static_cast<Array*>(obj->implementation);
	ASObject* ret;
	if(th->data.back().type==STACK_OBJECT)
		ret=th->data.back().data;
	else
		abort();
	th->data.pop_back();
	return ret;
}

ASFUNCTIONBODY(Array,_sort)
{
	LOG(LOG_NOT_IMPLEMENTED,"Array::sort not really implemented");
	return obj;
}

ASFUNCTIONBODY(Array,unshift)
{
	Array* th=static_cast<Array*>(obj->implementation);
	if(args->size()!=1)
	{
		LOG(LOG_ERROR,"Multiple unshift");
		abort();
	}
	th->data.insert(th->data.begin(),args->at(0));
	args->at(0)->incRef();
	return new Integer(th->size());
}

ASFUNCTIONBODY(Array,_push)
{
	Array* th=static_cast<Array*>(obj->implementation);
	if(args->size()!=1)
	{
		LOG(LOG_ERROR,"Multiple push");
		abort();
	}
	th->push(args->at(0));
	args->at(0)->incRef();
	return new Integer(th->size());
}

ASMovieClipLoader::ASMovieClipLoader()
{
}

ASFUNCTIONBODY(ASMovieClipLoader,constructor)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called MovieClipLoader constructor");
	return NULL;
}

ASFUNCTIONBODY(ASMovieClipLoader,addListener)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called MovieClipLoader::addListener");
	return NULL;
}

ASXML::ASXML()
{
	xml_buf=new char[1024*20];
	xml_index=0;
}

ASFUNCTIONBODY(ASXML,constructor)
{
	LOG(LOG_NOT_IMPLEMENTED,"Called XML constructor");
	return NULL;
}


size_t ASXML::write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	ASXML* th=(ASXML*)userp;
	memcpy(th->xml_buf+th->xml_index,buffer,size*nmemb);
	th->xml_index+=size*nmemb;
	return size*nmemb;
}

ASFUNCTIONBODY(ASXML,load)
{
	ASXML* th=static_cast<ASXML*>(obj);
	LOG(LOG_NOT_IMPLEMENTED,"Called ASXML::load " << args->at(0)->toString());
/*	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	string base("www.youtube.com");
	string url=base+args->at(0)->toString();
	if(curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, (string(url)).c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, obj);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	xmlDocPtr doc=xmlReadMemory(th->xml_buf,th->xml_index,(string(url)).c_str(),NULL,0);

	bool found;
	IFunction* on_load=obj->getVariableByName("onLoad",found)->toFunction();
	arguments a;
	a.push(new Integer(1));
	on_load->call(NULL,&a);*/
	return new Integer(1);
}

bool Array::isEqual(bool& ret, ASObject* r)
{
	if(r->getObjectType()!=T_ARRAY)
		ret=false;
	else
	{
		const Array* ra=static_cast<const Array*>(r->implementation);
		int size=data.size();
		if(size!=ra->size())
			ret=false;

		for(int i=0;i<size;i++)
		{
			if(data[i].type!=STACK_OBJECT)
				abort();
			if(!data[i].data->isEqual(ra->at(i)))
				ret=false;
		}
		ret=true;
	}
	return true;
}

bool Array::getVariableByMultiname_i(const multiname& name, intptr_t& out)
{
	int index=0;
	if(!isValidMultiname(name,index))
		return false;

	if(index<data.size())
	{
		switch(data[index].type)
		{
			case STACK_OBJECT:
			{
				assert(data[index].data!=NULL);
				if(data[index].data->getObjectType()==T_INTEGER)
				{
					Integer* i=static_cast<Integer*>(data[index].data);
					out=i->toInt();
				}
				else if(data[index].data->getObjectType()==T_NUMBER)
				{
					Number* i=static_cast<Number*>(data[index].data);
					out=i->toInt();
				}
				else
					abort();
				break;
			}
			case STACK_INT:
				out=data[index].data_i;
				break;
		}
		return true;
	}
	else
		return false;
}

bool Array::getVariableByMultiname(const multiname& name, ASObject*& out)
{
	int index=0;
	if(!isValidMultiname(name,index))
		return false;

	if(index<data.size())
	{
		switch(data[index].type)
		{
			case STACK_OBJECT:
				out=data[index].data;
				if(out==NULL)
				{
					out=new Undefined;
					data[index].data=out;
				}
				break;
			case STACK_INT:
				//cout << "Not efficent" << endl;
				out=abstract_i(data[index].data_i);
				out->fake_decRef();
				break;
		}
		return true;
	}
	else
		return false;
}

bool Array::setVariableByMultiname_i(const multiname& name, intptr_t value)
{
	int index=0;
	if(!isValidMultiname(name,index))
		return false;

	if(index>=data.capacity())
	{
		//Heuristic, we increse the array 20%
		int new_size=max(index+1,data.size()*2);
		data.reserve(new_size);
	}
	if(index>=data.size())
		resize(index+1);

	if(data[index].type==STACK_OBJECT && data[index].data)
		data[index].data->decRef();
	data[index].data_i=value;
	data[index].type=STACK_INT;

	return true;
}

bool Array::isValidMultiname(const multiname& name, int& index) const
{
	//First of all the multiname has to contain the null namespace
	//As the namespace vector is sorted, we check only the first one
	assert(name.ns.size()!=0);
	if(name.ns[0]!="")
		return false;

	index=0;
	int len;
	switch(name.name_type)
	{
		//We try to convert this to an index, otherwise bail out
		case multiname::NAME_STRING:
			len=name.name_s.len();
			assert(len);
			for(int i=0;i<len;i++)
			{
				if(name.name_s[i]<'0' || name.name_s[i]>'9')
					return false;

				index*=10;
				index+=(name.name_s[i]-'0');
			}
			break;
		//This is already an int, so its good enough
		case multiname::NAME_INT:
			index=name.name_i;
			break;
		case multiname::NAME_NUMBER:
			//TODO: check that this is really an integer
			index=name.name_d;
			break;
		default:
			abort();
	}
	return true;
}

bool Array::setVariableByMultiname(const multiname& name, ASObject* o)
{
	int index=0;
	if(!isValidMultiname(name,index))
		return false;

	if(index>=data.capacity())
	{
		//Heuristic, we increse the array 20%
		int new_size=max(index+1,data.size()*2);
		data.reserve(new_size);
	}
	if(index>=data.size())
		resize(index+1);

	if(data[index].type==STACK_OBJECT && data[index].data)
		data[index].data->decRef();

	if(o->getObjectType()==T_INTEGER)
	{
		Integer* i=static_cast<Integer*>(o);
		data[index].data_i=i->val;
		data[index].type=STACK_INT;
		o->decRef();
	}
	else
	{
		data[index].data=o;
		data[index].type=STACK_OBJECT;
	}
	return true;
}

bool Array::isValidQName(const tiny_string& name, const tiny_string& ns, int& index)
{
	if(ns!="")
		return false;
	assert(name.len()!=0);
	index=0;
	//First we try to convert the string name to an index, at the first non-digit
	//we bail out
	for(int i=0;i<name.len();i++)
	{
		if(!isdigit(name[i]))
			return false;

		index*=10;
		index+=(name[i]-'0');
	}
	return true;
}

bool Array::setVariableByQName(const tiny_string& name, const tiny_string& ns, ASObject* o)
{
	int index=0;
	if(!isValidQName(name,ns,index))
		return false;

	if(index>=data.capacity())
	{
		//Heuristic, we increse the array 20%
		int new_size=max(index+1,data.size()*2);
		data.reserve(new_size);
	}
	if(index>=data.size())
		resize(index+1);

	if(data[index].type==STACK_OBJECT && data[index].data)
		data[index].data->decRef();

	if(o->getObjectType()==T_INTEGER)
	{
		Integer* i=static_cast<Integer*>(o);
		data[index].data_i=i->val;
		data[index].type=STACK_INT;
		o->decRef();
	}
	else
	{
		data[index].data=o;
		data[index].type=STACK_OBJECT;
	}
	return true;
}

bool Array::getVariableByQName(const tiny_string& name, const tiny_string& ns, ASObject*& out)
{
	abort();
	return NULL;
/*	ASObject* ret;
	bool number=true;
	owner=NULL;
	for(int i=0;i<name.name.size();i++)
	{
		if(!isdigit(name.name[i]))
		{
			number=false;
			break;
		}

	}
	if(number)
	{
		int index=atoi(name.name.c_str());
		if(index<data.size())
		{
			if(data[index].type!=STACK_OBJECT)
				abort();
			ret=data[index].data;
			owner=this;
		}
	}

	if(!owner)
		ret=ASObject::getVariableByName(name,owner);

	return ret;*/

}

ASString::ASString()
{
	type=T_STRING;
	registerMethods();
}

ASString::ASString(const string& s):data(s)
{
	type=T_STRING;
	registerMethods();
}

ASString::ASString(const tiny_string& s):data(s.raw_buf())
{
	type=T_STRING;
	registerMethods();
}

ASString::ASString(const char* s):data(s)
{
	type=T_STRING;
	registerMethods();
}

ASFUNCTIONBODY(ASString,_getLength)
{
	ASString* th=static_cast<ASString*>(obj);
	return abstract_i(th->data.size());
}

void ASString::registerMethods()
{
	setVariableByQName("Call","",new Function(ASString::String));
	setVariableByQName("toString","",new Function(ASObject::_toString));
	setVariableByQName("split",AS3,new Function(split));
	setVariableByQName("replace",AS3,new Function(replace));
	setVariableByQName("concat",AS3,new Function(concat));
	setVariableByQName("indexOf",AS3,new Function(indexOf));
	setVariableByQName("charCodeAt",AS3,new Function(charCodeAt));
	setVariableByQName("slice",AS3,new Function(slice));
	setVariableByQName("toLowerCase",AS3,new Function(toLowerCase));
	setGetterByQName("length","",new Function(_getLength));
}

Array::~Array()
{
	for(int i=0;i<data.size();i++)
	{
		if(data[i].type==STACK_OBJECT && data[i].data)
			data[i].data->decRef();
	}
}

ASFUNCTIONBODY(ASString,split)
{
	ASString* th=static_cast<ASString*>(obj);
	Array* ret=Class<Array>::getInstanceS(true);
	ASObject* delimiter=args->at(0);
	if(delimiter->getObjectType()==T_STRING)
	{
		ASString* del=static_cast<ASString*>(delimiter);
		int start=0;
		do
		{
			int match=th->data.find(del->data,start);
			if(match==-1)
				match=th->data.size();
			ASString* s=new ASString(th->data.substr(start,match));
			ret->push(s);
			start=match+del->data.size();
		}
		while(start<th->data.size());
	}
	else
		abort();

	return ret->obj;
}

ASFUNCTIONBODY(ASString,String)
{
	if(args->at(0)->getObjectType()==T_INTEGER)
	{
		Integer* n=static_cast<Integer*>(args->at(0));
		ostringstream oss;
		oss << setprecision(8) << fixed << *n;

		return new ASString(oss.str());
	}
	else if(args->at(0)->getObjectType()==T_NUMBER)
	{
		Number* n=static_cast<Number*>(args->at(0));
		ostringstream oss;
		oss << setprecision(8) << fixed << *n;

		return new ASString(oss.str());
	}
	else if(args->at(0)->getObjectType()==T_ARRAY)
	{
		cout << "array" << endl;
		cout << args->at(0)->toString() << endl;
		return new ASString(args->at(0)->toString());
	}
	else if(args->at(0)->getObjectType()==T_UNDEFINED)
	{
		return new ASString("undefined");
	}
	else
	{
		LOG(LOG_CALLS,"Cannot convert " << args->at(0)->getObjectType() << " to String");
		abort();
	}
}

bool Array::toString(tiny_string& ret)
{
	ret=toString();
	return true;
}

tiny_string Array::toString() const
{
	string ret;
	for(int i=0;i<data.size();i++)
	{
		if(data[i].type!=STACK_OBJECT)
			abort();
		ret+=data[i].data->toString().raw_buf();
		if(i!=data.size()-1)
			ret+=',';
	}
	return ret.c_str();
}

tiny_string Boolean::toString() const
{
	return (val)?"true":"false";
}

tiny_string ASString::toString() const
{
	return data.c_str();
}

double ASString::toNumber() const
{
	LOG(LOG_ERROR,"Cannot convert string " << data << " to float");
	return 0;
}

ASFUNCTIONBODY(Undefined,call)
{
	LOG(LOG_CALLS,"Undefined function");
	return NULL;
}

tiny_string Undefined::toString() const
{
	return "null";
}

bool ASString::isEqual(ASObject* r)
{
	if(r->getObjectType()==T_STRING)
	{
		const ASString* s=static_cast<const ASString*>(r);
		if(s->data==data)
			return true;
		else
			return false;
	}
	else
		return false;
}

bool Boolean::isEqual(ASObject* r)
{
	if(r->getObjectType()==T_BOOLEAN)
	{
		const Boolean* b=static_cast<const Boolean*>(r);
		return b->val==val;
	}
	else
	{
		return ASObject::isEqual(r);
	}
}

bool Undefined::isEqual(ASObject* r)
{
	if(r->getObjectType()==T_UNDEFINED)
		return true;
	if(r->getObjectType()==T_NULL)
		return true;
	else
		return false;
}

Undefined::Undefined()
{
	type=T_UNDEFINED;
//	setVariableByName(".Call",new Function(call));
}

bool Number::isEqual(ASObject* o)
{
	if(o->getObjectType()==T_INTEGER)
		return val==o->toNumber();
	else if(o->getObjectType()==T_NUMBER)
		return val==o->toNumber();
	else
	{
		return ASObject::isLess(o);
	}
}

bool Number::isGreater(ASObject* o)
{
	if(o->getObjectType()==T_INTEGER)
	{
		const Integer* i=static_cast<const Integer*>(o);
		return val>i->val;
	}
	else if(o->getObjectType()==T_NUMBER)
	{
		const Number* i=static_cast<const Number*>(o);
		return val>i->val;
	}
	else
	{
		return ASObject::isGreater(o);
	}
}

bool Number::isLess(ASObject* o)
{
	if(o->getObjectType()==T_INTEGER)
	{
		const Integer* i=static_cast<const Integer*>(o);
		return val<i->val;
	}
	else if(o->getObjectType()==T_NUMBER)
	{
		const Number* i=static_cast<const Number*>(o);
		return val<i->val;
	}
	else
	{
		return ASObject::isLess(o);
	}
}

tiny_string Number::toString() const
{
	char buf[20];
	snprintf(buf,20,"%g",val);
	return buf;
}

Date::Date():year(-1),month(-1),date(-1),hour(-1),minute(-1),second(-1),millisecond(-1)
{
}

void Date::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

ASFUNCTIONBODY(Date,_constructor)
{
	Date* th=static_cast<Date*>(obj->implementation);
	obj->setVariableByQName("getTimezoneOffset","",new Function(getTimezoneOffset));
	obj->setVariableByQName("valueOf","",new Function(valueOf));
	obj->setVariableByQName("getTime",AS3,new Function(getTime));
	obj->setVariableByQName("getFullYear","",new Function(getFullYear));
	//th->setVariableByName(Qname(AS3,"getFullYear"),new Function(getFullYear));
	obj->setVariableByQName("getHours",AS3,new Function(getHours));
	obj->setVariableByQName("getMinutes",AS3,new Function(getMinutes));
	obj->setVariableByQName("getSeconds",AS3,new Function(getMinutes));
	obj->setVariableByQName("toString",AS3,new Function(ASObject::_toString));
	th->year=1969;
	th->month=1;
	th->date=1;
	th->hour=0;
	th->minute=0;
	th->second=0;
	th->millisecond=0;
	return NULL;
}

ASFUNCTIONBODY(Date,getTimezoneOffset)
{
	LOG(LOG_NOT_IMPLEMENTED,"getTimezoneOffset");
	return new Number(120);
}

ASFUNCTIONBODY(Date,getFullYear)
{
	Date* th=static_cast<Date*>(obj->implementation);
	return new Number(th->year);
}

ASFUNCTIONBODY(Date,getHours)
{
	Date* th=static_cast<Date*>(obj->implementation);
	return new Number(th->hour);
}

ASFUNCTIONBODY(Date,getMinutes)
{
	Date* th=static_cast<Date*>(obj->implementation);
	return new Number(th->minute);
}

ASFUNCTIONBODY(Date,getTime)
{
	Date* th=static_cast<Date*>(obj->implementation);
	return new Number(th->toInt());
}

ASFUNCTIONBODY(Date,valueOf)
{
	Date* th=static_cast<Date*>(obj->implementation);
	return new Number(th->toInt());
}

bool Date::toInt(int& ret)
{
	ret=toInt();
	return true;
}

int Date::toInt() const
{
	int ret=0;
	//TODO: leap year
	ret+=(year-1990)*365*24*3600*1000;
	//TODO: month length
	ret+=(month-1)*30*24*3600*1000;
	ret+=(date-1)*24*3600*1000;
	ret+=hour*3600*1000;
	ret+=minute*60*1000;
	ret+=second*1000;
	ret+=millisecond;
	return ret;
}

bool Date::toString(tiny_string& ret)
{
	ret=toString();
	return true;
}

tiny_string Date::toString() const
{
	return "Wed Dec 31 16:00:00 GMT-0800 1969";
}

tiny_string Function_Object::toString() const
{
	return "function Function() {}";
}

IFunction* SyntheticFunction::toFunction()
{
	return this;
}

IFunction* Function::toFunction()
{
	return this;
}

SyntheticFunction::SyntheticFunction(method_info* m):mi(m)
{
//	class_index=-2;
	val=m->synt_method();
}

ASObject* SyntheticFunction::fast_call(ASObject* obj, ASObject** args, int numArgs, int level)
{
	if(mi->needsArgs())
		abort();

	int args_len=mi->numArgs();
	int passedToLocals=min(numArgs,args_len);
	int passedToRest=(numArgs > args_len)?(numArgs-mi->numArgs()):0;
	call_context* cc=new call_context(mi,level,args,passedToLocals);
	int i=passedToLocals;
	cc->scope_stack=func_scope;
	for(int i=0;i<func_scope.size();i++)
		func_scope[i]->incRef();

	if(bound)
	{
		LOG(LOG_CALLS,"Calling with closure " << this);
		obj=closure_this;
	}

	cc->locals[0]=obj;

	//Fixup missing parameters
	int missing_params=args_len-i;
	assert(missing_params<=mi->option_count);
	int starting_options=mi->option_count-missing_params;

	for(int j=starting_options;j<mi->option_count;j++)
	{
		cc->locals[i+1]=mi->getOptional(j);
		i++;
	}
	
	assert(i==mi->numArgs());

	/*for(int i=numArgs;i<mi->numArgs();i++)
	{
		cout << "filling arg " << i << endl;
		cc->locals[i+1]=new Undefined;
	}*/

	if(mi->needsRest()) //TODO
	{
		assert(passedToRest==0);
		Array* rest=Class<Array>::getInstanceS(true);
		cc->locals[i+1]=rest->obj;
	}
	obj->incRef();
	ASObject* ret=val(obj,NULL,cc);

	delete cc;
	return ret;
}

ASObject* SyntheticFunction::call(ASObject* obj, arguments* args, int level)
{
	if(val==NULL)
	{
		LOG(LOG_NOT_IMPLEMENTED,"Not initialized function");
		return NULL;
	}

	call_context* cc=new call_context(mi, level);
	cc->scope_stack=func_scope;
	for(int i=0;i<func_scope.size();i++)
		func_scope[i]->incRef();

	if(bound)
	{
		LOG(LOG_CALLS,"Calling with closure " << this);
		obj=closure_this;
	}

	cc->locals[0]=obj;
	cc->locals[0]->incRef();
	int i=0;
	int args_len=mi->numArgs();
	if(args)
	{
		if(args->size()>mi->numArgs())
		{
			assert(mi->needsRest());
		}
		for(i;i<min(args->size(),args_len);i++)
		{
			cc->locals[i+1]=args->at(i);
			cc->locals[i+1]->incRef();
		}
	}

	//Fixup missing parameters
	int missing_params=args_len-i;
	assert(missing_params<=mi->option_count);
	int starting_options=mi->option_count-missing_params;

	for(int j=starting_options;j<mi->option_count;j++)
	{
		cc->locals[i+1]=mi->getOptional(j);
		i++;
	}
	
	assert(i==mi->numArgs());

/*	for(i;i<mi->numArgs();i++)
	{
		cout << "filling arg " << i << endl;
		cc->locals[i+1]=new Undefined;
	}*/

	if(mi->needsRest()) //TODO
	{
		Array* rest=Class<Array>::getInstanceS();
		rest->_constructor(rest->obj,NULL);
		cc->locals[mi->numArgs()+1]=rest->obj;
		/*llvm::Value* rest=Builder.CreateCall(ex->FindFunctionNamed("createRest"));
		constant = llvm::ConstantInt::get(int_type, param_count+1);
		t=Builder.CreateGEP(locals,constant);
		Builder.CreateStore(rest,t);*/
	}

	obj->incRef();
	ASObject* ret=val(obj,args,cc);

	delete cc;
	return ret;
}

ASObject* Function::fast_call(ASObject* obj, ASObject** args,int num_args, int level)
{
	arguments arg(num_args,false);
	for(int i=0;i<num_args;i++)
		arg.set(i,args[i]);
	return call(obj,&arg,level);
}

ASObject* Function::call(ASObject* obj, arguments* args, int level)
{
	if(!bound)
		return val(obj,args);
	else
	{
		LOG(LOG_CALLS,"Calling with closure " << this);
		LOG(LOG_CALLS,"args 0 " << args->at(0));
		return val(closure_this,args);
	}
}

Math::Math()
{
	setVariableByQName("PI","",new Number(M_PI));
	setVariableByQName("sqrt","",new Function(sqrt));
	setVariableByQName("atan2","",new Function(atan2));
	setVariableByQName("max","",new Function(_max));
	setVariableByQName("min","",new Function(_min));
	setVariableByQName("abs","",new Function(abs));
	setVariableByQName("sin","",new Function(sin));
	setVariableByQName("cos","",new Function(cos));
	setVariableByQName("floor","",new Function(floor));
	setVariableByQName("round","",new Function(round));
	setVariableByQName("random","",new Function(random));
	setVariableByQName("pow","",new Function(pow));
}

ASFUNCTIONBODY(Math,atan2)
{
	double n1=args->at(0)->toNumber();
	double n2=args->at(1)->toNumber();
	return abstract_d(::atan2(n1,n2));
}

ASFUNCTIONBODY(Math,_max)
{
	double n1=args->at(0)->toNumber();
	double n2=args->at(1)->toNumber();
	return abstract_d(dmax(n1,n2));
}

ASFUNCTIONBODY(Math,_min)
{
	double n1=args->at(0)->toNumber();
	double n2=args->at(1)->toNumber();
	return abstract_d(dmin(n1,n2));
}

ASFUNCTIONBODY(Math,cos)
{
	//Angle is in radians
	double n=args->at(0)->toNumber();
	return abstract_d(::cos(n));
}

ASFUNCTIONBODY(Math,sin)
{
	//Angle is in radians
	double n=args->at(0)->toNumber();
	return abstract_d(::sin(n));
}

ASFUNCTIONBODY(Math,abs)
{
	double n=args->at(0)->toNumber();
	return abstract_d(::abs(n));
}

ASFUNCTIONBODY(Math,floor)
{
	double n=args->at(0)->toNumber();
	return abstract_i(::floor(n));
}

ASFUNCTIONBODY(Math,round)
{
	double n=args->at(0)->toNumber();
	return abstract_i(::round(n));
}

ASFUNCTIONBODY(Math,sqrt)
{
	double n=args->at(0)->toNumber();
	return abstract_d(::sqrt(n));
}

ASFUNCTIONBODY(Math,pow)
{
	double x=args->at(0)->toNumber();
	double y=args->at(1)->toNumber();
	return abstract_d(::pow(x,y));
}

ASFUNCTIONBODY(Math,random)
{
	double ret=rand();
	ret/=RAND_MAX;
	return abstract_d(ret);
}

tiny_string Null::toString() const
{
	return "null";
}

bool Null::isEqual(ASObject* r)
{
	if(r->getObjectType()==T_NULL)
		return true;
	else if(r->getObjectType()==T_UNDEFINED)
		return true;
	else
		return false;
}

RegExp::RegExp()
{
}

void RegExp::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

ASFUNCTIONBODY(RegExp,_constructor)
{
	RegExp* th=static_cast<RegExp*>(obj->implementation);
	th->re=args->at(0)->toString().raw_buf();
	if(args->size()>1)
		th->flags=args->at(1)->toString().raw_buf();
	obj->setVariableByQName("exec",AS3,new Function(exec));
	return NULL;
}

ASFUNCTIONBODY(RegExp,exec)
{
	RegExp* th=static_cast<RegExp*>(obj->implementation);
	cout << "Returning tracer2" <<endl;
	return new DebugTracer("RegExp::exec");
}

ASFUNCTIONBODY(ASString,slice)
{
	ASString* th=static_cast<ASString*>(obj);
	int startIndex=args->at(0)->toInt();
	int endIndex=args->at(1)->toInt();
	return new ASString(th->data.substr(startIndex,endIndex));
}

ASFUNCTIONBODY(ASString,charCodeAt)
{
	LOG(LOG_NOT_IMPLEMENTED,"ASString::charCodeAt not really implemented");
	ASString* th=static_cast<ASString*>(obj);
	int index=args->at(0)->toInt();
	return new Integer(th->data[index]);
}

ASFUNCTIONBODY(ASString,indexOf)
{
	LOG(LOG_NOT_IMPLEMENTED,"ASString::indexOf not really implemented");
	ASString* th=static_cast<ASString*>(obj);
	return new Integer(-1);
}

ASFUNCTIONBODY(ASString,toLowerCase)
{
	ASString* th=static_cast<ASString*>(obj);
	ASString* ret=new ASString;
	ret->data=th->data;
	transform(th->data.begin(), th->data.end(), ret->data.begin(), ::tolower);
	return ret;
}

ASFUNCTIONBODY(ASString,replace)
{
	LOG(LOG_NOT_IMPLEMENTED,"ASString::replace not really implemented");
	const ASString* th=static_cast<const ASString*>(obj);
	ASString* ret=new ASString(th->data);

	assert(args->size()==2 && args->at(1)->getObjectType()==T_STRING);

	if(args->at(0)->prototype!=Class<RegExp>::getClass())
		abort();

	RegExp* re=static_cast<RegExp*>(args->at(0)->implementation);
	assert(re->re.size()==1 && re->flags.size()==1);

	bool g=(re->flags[0]=='g')?true:false;
	const tiny_string& replaceWith=args->at(1)->toString();
	assert(replaceWith.len()==1);

	for(int i=0;i<th->data.size();i++)
	{
		if(th->data[i]==re->re[0])
		{
			ret->data[i]=replaceWith[0];
			if(!g)
				break;
		}
	}

	return ret;
}

ASFUNCTIONBODY(ASString,concat)
{
	LOG(LOG_NOT_IMPLEMENTED,"ASString::concat not really implemented");
	ASString* th=static_cast<ASString*>(obj);
	th->data+=args->at(0)->toString().raw_buf();
	th->incRef();
	return th;
}

Class_base::~Class_base()
{
	if(constructor)
		constructor->decRef();
}

void Class_base::addImplementedInterface(const multiname& i)
{
	interfaces.push_back(i);
}

IInterface* Class_inherit::getInstance(bool construct)
{
	//TODO: Ask our super for the interface to ret
	assert(super);
	IInterface* ret=super->getInstance(construct);
	ret->obj->max_level=max_level;
	ret->obj->prototype->decRef();
	ret->obj->prototype=this;
	ret->obj->actualPrototype=this;
	//As we are the prototype we should incRef ourself
	incRef();
	return ret;
}

void Class_inherit::buildInstanceTraits(ASObject* o) const
{
	assert(class_index!=-1);
	//The class is declared in the script and has an index
	LOG(LOG_CALLS,"Building instance traits");

	//To insert the trait in the rigth level we have to change the max_level
	int oldlevel=o->max_level;
	o->max_level=max_level;

	context->buildInstanceTraits(o,class_index);

	o->max_level=oldlevel;
}

Class_object* Class_object::getClass()
{
	//We check if we are registered in the class map
	//if not we register ourselves (see also Class<T>::getClass)
	std::map<tiny_string, Class_base*>::iterator it=sys->classes.find("Class");
	if(it==sys->classes.end()) //This class is not yet in the map, create it
	{
		Class_object* ret=new Class_object();
		sys->classes.insert(std::make_pair("Class",ret));
		return ret;
	}
	else
		return static_cast<Class_object*>(it->second);
}

void ASQName::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

ASFUNCTIONBODY(ASQName,_constructor)
{
	ASQName* th=static_cast<ASQName*>(obj->implementation);
	if(args->size()!=2)
		abort();

	assert(args->at(0)->getObjectType()==T_STRING || args->at(0)->getObjectType()==T_NAMESPACE);
	assert(args->at(1)->getObjectType()==T_STRING);

	switch(args->at(0)->getObjectType())
	{
		case T_STRING:
		{
			ASString* s=static_cast<ASString*>(args->at(0));
			th->uri=s->data;
			break;
		}
		case T_NAMESPACE:
		{
			Namespace* n=static_cast<Namespace*>(args->at(0)->implementation);
			th->uri=n->uri;
			break;
		}
	}
	th->local_name=args->at(1)->toString();
}

void Namespace::sinit(Class_base* c)
{
	assert(c->constructor==NULL);
	c->constructor=new Function(_constructor);
}

ASFUNCTIONBODY(Namespace,_constructor)
{
	abort();
}

bool lightspark::isSubclass(ASObject* obj, const Class_base* c)
{
	Class_base* cur=obj->prototype;
	while(cur)
	{
		if(cur==c)
			return true;
		cur=cur->super;
	}
	return false;
}
