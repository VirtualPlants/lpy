/* ---------------------------------------------------------------------------
#
#       L-Py: L-systems in Python
#
#       Copyright 2003-2008 UMR Cirad/Inria/Inra Dap - Virtual Plant Team
#
#       File author(s): F. Boudon (frederic.boudon@cirad.fr)
#
# ---------------------------------------------------------------------------
#
#                      GNU General Public Licence
#
#       This program is free software; you can redistribute it and/or
#       modify it under the terms of the GNU General Public License as
#       published by the Free Software Foundation; either version 2 of
#       the License, or (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public
#       License along with this program; see the file COPYING. If not,
#       write to the Free Software Foundation, Inc., 59
#       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# ---------------------------------------------------------------------------
*/

#ifndef __PGL_LSYSCONTEXT_H__
#define __PGL_LSYSCONTEXT_H__

#include "axialtree.h"
#include "lsysoptions.h"
#include "paramproduction.h"
#include <plantgl/algo/modelling/pglturtle.h>
#include <plantgl/tool/util_hashset.h>
#include <QtCore/QReadWriteLock>

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

class LPY_API LsysContext {
public:
  friend class Lsystem;
  friend class ModuleVTable;

  /** string value of python variable containing lsystem informations. */
  static const std::string InitialisationFunctionName;
  static const std::string AxiomVariable;
  static const std::string DerivationLengthVariable;
  static const std::string DecompositionMaxDepthVariable;
  static const std::string HomomorphismMaxDepthVariable;
  static double DefaultAnimationTimeStep;

  /** Constructor */
  LsysContext();

  /** Destructor */
  virtual ~LsysContext();

  /** clear context. Set python namespace to default. Keep __builtin__, pylsystems and __filename__ object */
  void clear();
  /** Test whether namespace is empty */
  bool empty() const;

  /** consider and ignore of symbol*/
  void consider(const std::string& modules);
  void ignore(const std::string& modules);
  bool isConsidered(const std::string& module) const;
  bool isIgnored(const std::string& module) const;
  bool isConsidered(const Module& module) const;
  bool isIgnored(const Module& module) const;
  bool ignoring() const { return __ignore_method; }
  std::string keyword() const;

  /** string representation */
  std::string str() const ;

  /** The Start, End, StartEach and EndEach execution */
  void start();
  void end();
  void end(AxialTree&);
  void end(AxialTree&, const PGL::ScenePtr&);
  void startEach();
  void endEach();
  void endEach(AxialTree&);
  void endEach(AxialTree&, const PGL::ScenePtr&);

  /** The Start, End, StartEach and EndEach initialisation */
  void setStart(boost::python::object func);
  void setEnd(boost::python::object func);
  void setStartEach(boost::python::object func);
  void setEndEach(boost::python::object func);
  void check_init_functions();
  inline size_t getEndEachNbArgs() const { return  __nbargs_of_endeach; }
  inline size_t getEndNbArgs() const { return  __nbargs_of_end; }


  /// initialise context using python function in namespace.
  bool initialise();
  size_t initialiseFrom(const std::string& lcode);

  /** compilation of code into the python namespace */
  void compile(const std::string&)  ;
  boost::python::object  evaluate(const std::string&)  ;
  boost::python::object  try_evaluate(const std::string&)  ;

  virtual boost::python::object compile(const std::string& name, const std::string& code)  ;

  /** application of a function */
  void func(const std::string& funcname);

  /** access to value of the python namespace */
  int readInt(const std::string&) ;
  float readReal(const std::string&)  ;

  /** python namespace management */
  virtual void clearNamespace();
  virtual void updateNamespace(const boost::python::dict&);
  virtual void getNamespace(boost::python::dict&) const;

  /** access to python object of the namespace */
  virtual bool hasObject(const std::string& name) const;
  virtual boost::python::object getObject(const std::string& name, const boost::python::object& defaultvalue = boost::python::object()) const;
  virtual void setObject(const std::string& name, 
				 const boost::python::object&);
  virtual void delObject(const std::string& name) ;
  bool copyObject(const std::string& name, LsysContext * sourceContext) ;
  
  /** make current or disable a context */
  void makeCurrent();
  bool isCurrent() const ;
  void done() ;

  /** static functions to access context */
  static inline LsysContext * currentContext() { return current(); }
  static LsysContext * current();
  static inline LsysContext * globalContext() { return global(); }
  static LsysContext * global();
  static LsysContext * defaultContext() ;
  static void cleanContexts();

  /** control of the direction of next iteration */
  inline void backward() { __direction = eBackward; }
  inline void forward() { __direction = eForward; }
  inline bool isForward() { return __direction == eForward; }
  inline eDirection getDirection() const { return __direction; }

  /** selection of group of rules */
  inline void useGroup(size_t gid) { __group = gid; }
  inline size_t getGroup() const  { return __group; }

  /** control of frame display */
  inline void frameDisplay(bool enabled) { __frameDisplay = enabled; }
  inline bool isFrameDisplayed() const  { return __frameDisplay; }

  /** iterative production */
  inline void nproduce(const AxialTree& prod)
  { __nproduction.append(prod); }

  inline void nproduce(const boost::python::list& prod)
  { __nproduction.append(AxialTree(prod)); }

  inline void reset_nproduction() { __nproduction.clear(); }
  inline AxialTree get_nproduction() const { return __nproduction; }
  inline void set_nproduction(const AxialTree& prod) { __nproduction = prod; }

  /** parametric production */
  inline size_t add_pproduction(const ParametricProduction& pprod)
  { size_t id = __paramproductions.size(); __paramproductions.push_back(pprod); return id; }

  inline const ParametricProduction& get_pproduction(size_t id) const
  { lpyassert(id <__paramproductions.size()); return __paramproductions[id]; }

  inline const ParametricProductionList& get_pproductions() const 
  { return __paramproductions; }

  inline AxialTree generate(size_t pprod_id, const bp::list& args)
  { lpyassert(pprod_id<__paramproductions.size()); return __paramproductions[pprod_id].generate(args); }


  inline AxialTree generate(const bp::tuple& args)
  { size_t pprod_id = bp::extract<size_t>(args[0])();
    lpyassert(pprod_id<__paramproductions.size()); 
	return __paramproductions[pprod_id].generate(args); }

  inline void pproduce(size_t pprod_id, const bp::list& args)
  { __nproduction.append(generate(pprod_id,args)); }

  inline void pproduce(const bp::tuple& args)
  { __nproduction.append(generate(args)); }

  /** animation time step property */
  double get_animation_timestep();
  void set_animation_timestep(double value);
  bool is_animation_timestep_to_default();

  inline bool isAnimationEnabled() const { return __animation_enabled; }
  inline void setAnimationEnabled(bool enabled) { __animation_enabled = enabled; }

  /** Specify if the selection check is required */
  bool isSelectionRequired() const;
  void setSelectionRequired(bool enabled);

  /// Specify whether a warning should be made if found sharp module
  inline bool warnWithSharpModule() const { return __warn_with_sharp_module; }
  void setWarnWithSharpModule(bool);

  /** Turtles and interpretation structures */
  PGL(PglTurtle) turtle;
  PGL(Turtle)    envturtle;

  /** Context options */
  LsysOptions options;

  /** module declaration. */
  void declare(const std::string& modules);
  inline void declare(ModuleClassPtr module)
  { __modules.push_back(module); }

  void undeclare(const std::string& modules);
  void undeclare(ModuleClassPtr module);
  bool isDeclared(const std::string& module);
  bool isDeclared(ModuleClassPtr module);
  ModuleClassList declaredModules() const { return __modules; }
  void declareAlias(const std::string& aliasName, ModuleClassPtr module);

  void setModuleScale(const std::string& modules, int scale);


  /// return if no matching
  bool return_if_no_matching;
  inline void setReturnIfNoMatching(bool enabled) { return_if_no_matching = enabled; }

  /// optimization level
  static const int DEFAULT_OPTIMIZATION_LEVEL;
  int optimizationLevel;
  inline void setOptimizationLevel(int level) { optimizationLevel = level; }

  /** early return */
  void enableEarlyReturn(bool val) ;
  bool isEarlyReturnEnabled() ;
  inline void stop() { enableEarlyReturn(true); }

  
  /** Iteration number property. Only set by Lsystem. Access by all other. */
public:
  size_t getIterationNb();
protected:
  void setIterationNb(size_t) ;

protected:
  void controlMethod(const std::string&, AxialTree&);
  void controlMethod(const std::string&, AxialTree&, const PGL::ScenePtr&);

  /// initialise context using python function in namespace.
  bool __initialise();
  size_t __initialiseFrom(const std::string& lcode);

  /** Event when context is made current, release, pushed or restore */
  virtual void currentEvent();
  virtual void doneEvent();
  virtual void pushedEvent(LsysContext * newEvent);
  virtual void restoreEvent(LsysContext * previousEvent);

  /// protected copy constructor.
  LsysContext(const LsysContext& lsys);
  LsysContext& operator=(const LsysContext& lsys);

  /// protected access to python namespace. To be redefined.
  virtual PyObject * Namespace()  const { return NULL; };

  /// init options
  void init_options();

  /// attributes for ignore and consider
  typedef pgl_hash_map<size_t,ModuleClassPtr> ModuleClassSet;
  ModuleClassSet __keyword;
  bool __ignore_method;

  /// attributes for module declaration
  ModuleClassList __modules;
  ModuleVTableList __modulesvtables;

  typedef pgl_hash_map_string<ModuleClassPtr> AliasSet;
  AliasSet __aliases;

  /// next iteration control
  eDirection __direction;
  size_t __group;
  bool __frameDisplay;

  /// iterative production
  AxialTree __nproduction;

  /// selection required property
  bool __selection_required;

  /// Warn if found sharp module
  bool __warn_with_sharp_module;

  /// animation step property and its mutex
  double __animation_step;
  QReadWriteLock __animation_step_mutex;
  /// animation property
  bool __animation_enabled;

  /// iteration nb property and its mutex
  size_t __iteration_nb;
  QReadWriteLock __iteration_nb_lock;

  size_t __nbargs_of_endeach;
  size_t __nbargs_of_end;

  // list of parametric production
  ParametricProductionList __paramproductions;

  // For multithreaded appli, allow to set an early_return
  bool __early_return;
  QReadWriteLock __early_return_mutex;
};

/*---------------------------------------------------------------------------*/

class LPY_API LocalContext : public LsysContext {
public:
  LocalContext(bool with_initialisation = true);
  ~LocalContext();

  boost::python::dict& getNamespace() { return __namespace; }
  const boost::python::dict& getNamespace() const { return __namespace; }

  virtual void clearNamespace();
  virtual void updateNamespace(const boost::python::dict&);
  virtual void getNamespace(boost::python::dict&) const;

  virtual bool hasObject(const std::string& name) const;
  virtual boost::python::object getObject(const std::string& name, const boost::python::object& defaultvalue = boost::python::object()) const;
  virtual void setObject(const std::string& name, 
				 const boost::python::object&);
  virtual void delObject(const std::string& name) ;

protected:
  void initialisation();
  virtual PyObject * Namespace() const ;

  boost::python::dict __namespace;
};

/*---------------------------------------------------------------------------*/

class LPY_API GlobalContext : public LsysContext {
public:
  GlobalContext();
  ~GlobalContext();

  virtual void clearNamespace();
  virtual void updateNamespace(const boost::python::dict&);
  virtual void getNamespace(boost::python::dict&) const;

  virtual bool hasObject(const std::string& name) const;
  virtual boost::python::object getObject(const std::string& name, const boost::python::object& defaultvalue = boost::python::object()) const;
  virtual void setObject(const std::string& name, 
				 const boost::python::object&);
  virtual void delObject(const std::string& name) ;

  virtual boost::python::object compile(const std::string& name, const std::string& code)  ;

  static boost::python::object getFunctionRepr();
protected:
  virtual PyObject * Namespace() const ;

  boost::python::handle<> __namespace;
  boost::python::dict __local_namespace;
  static boost::python::object __reprFunc;

};

/*---------------------------------------------------------------------------*/

inline void LPY_API consider(const std::string& modules)
{ LsysContext::currentContext()->consider(modules); }

inline void LPY_API ignore(const std::string& modules)
{ LsysContext::currentContext()->ignore(modules); }

inline void LPY_API nproduce(const AxialTree& prod)
{ LsysContext::currentContext()->nproduce(prod); }

inline void LPY_API nproduce(const boost::python::list& prod)
{ LsysContext::currentContext()->nproduce(prod); }

inline void LPY_API nproduce(const std::string& modules)
{ LsysContext::currentContext()->nproduce(AxialTree(modules)); }

inline void LPY_API pproduce(size_t id, const boost::python::list& args)
{ LsysContext::currentContext()->pproduce(id,args); }

inline void LPY_API pproduce(const boost::python::tuple& args)
{ LsysContext::currentContext()->pproduce(args); }

inline void LPY_API useGroup(size_t gid)
{ LsysContext::currentContext()->useGroup(gid); }

inline size_t LPY_API getGroup()
{ return LsysContext::currentContext()->getGroup(); }

inline void LPY_API frameDisplay(bool enabled)
{ LsysContext::currentContext()->frameDisplay(enabled); }

inline bool LPY_API isFrameDisplayed()
{ return LsysContext::currentContext()->isFrameDisplayed(); }

inline void LPY_API setSelectionRequired(bool enabled)
{ LsysContext::currentContext()->setSelectionRequired(enabled); }

inline bool LPY_API isSelectionRequired()
{ return LsysContext::currentContext()->isSelectionRequired(); }

inline size_t LPY_API getIterationNb()
{ return LsysContext::currentContext()->getIterationNb(); }

inline void LPY_API declare(const std::string& modules)
{ LsysContext::currentContext()->declare(modules); }

inline void LPY_API undeclare(const std::string& modules)
{ LsysContext::currentContext()->undeclare(modules); }

inline void LPY_API isDeclared(const std::string& module)
{ LsysContext::currentContext()->isDeclared(module); }

inline bool LPY_API isAnimationEnabled()
{ return LsysContext::currentContext()->isAnimationEnabled(); }

inline void LPY_API Stop()
{ return LsysContext::currentContext()->stop(); }

/*---------------------------------------------------------------------------*/

struct ContextMaintainer {
    bool is_set;
    LsysContext * context;

    ContextMaintainer(LsysContext * _context) : 
        is_set(!_context->isCurrent()), context(_context)
    { 
		if (is_set) context->makeCurrent(); 
    }

    ~ContextMaintainer() { if (is_set) context->done();  }
};

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE

/*---------------------------------------------------------------------------*/

#endif
