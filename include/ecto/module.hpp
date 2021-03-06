/*
 * Copyright (c) 2011, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <ecto/tendril.hpp>
#include <ecto/tendrils.hpp>
#include <ecto/util.hpp>

#include <map>

namespace ecto
{

  /**
   * \brief Return values for modules' process functions. These
   * are appropriate for non exceptional behavior.
   */
  enum ReturnCode
  {
    OK = 0, //!< Everything A OK.
    QUIT = 1,
  //!< Explicit quit now.
  };

  /**
   * \brief ecto::module is the non virtual interface to the basic building
   * block of ecto graphs.  This interface should never be the parent of a
   * client modules, but may be used for polymorphic access to client modules.
   *
   * Clients should expose their code to this interface through
   * ecto::wrap, or ecto::create_module<T>().
   *
   * For a client's module to satisfy the ecto::module idium, it must
   * look similar to the following definition.
   * @code
   struct MyModule
   {
   //called first thing, the user should declare their parameters in this
   //free standing function.
   static void declare_params(tendrils& params);
   //declare inputs and outputs here. The parameters may be used to
   //determine the io
   static void declare_io(const tendrils& params, tendrils& in, tendrils& out);
   //called right after allocation of the module, exactly once.
   void configure(tendrils& params);
   //called at every execution of the graph
   int process(const tendrils& in, tendrils& out);
   //called right before the destructor of the module, a good place to do
   //critical cleanup work.
   void destroy();
   };
   * @endcode
   *
   * It is important to note that all functions have are optional and they all have
   * default implementations.
   */
  struct module: boost::noncopyable
  {
    typedef boost::shared_ptr<module> ptr; //!< A convenience pointer typedef

    module();
    virtual ~module();

    /**
     * \brief Dispatches parameter declaration code. After this code, the parameters
     * for the module will be set to their defaults.
     */
    void declare_params();
    /**
     * \brief Dispatches input/output declaration code.  It is assumed that the parameters
     * have been declared before this is called, so that inputs and outputs may be dependent
     * on those parameters.
     */
    void declare_io();

    /**
     * \brief Given initialized parameters,inputs, and outputs, this will dispatch the client
     * configuration code.  This will allocated an instace of the clients module, so this
     * should not be called during introspection.
     */
    void configure();

    /**
     * \brief Dispatches the process function for the client module.  This should only
     * be called from one thread at a time.
     *
     * Also, this function may throw exceptions...
     *
     * @return A return code, ecto::OK , or 0 means all is ok. Anything non zero should be considered an
     * exit signal.
     */
    ReturnCode process();

    /**
     * \brief This should be called at the end of life for the module, and signals immenent destruction.
     *
     * Will dispatch the clients destroy code. After this call, do not call any other functions.
     */
    void destroy();

    /**
     * \brief Grab the name of the child class.
     * @return A human readable non mangled name for the client class.
     */
    std::string name() const;

    /**
     * \brief Generate an Restructured Text doc string for the module. Includes documentation for all parameters,
     * inputs, outputs.
     * @param doc The highest level documentation for the module.
     * @return A nicely formatted doc string.
     */
    std::string gen_doc(const std::string& doc = "A module...") const;

    ptr clone() const;

    tendrils parameters; //!< Parameters
    tendrils inputs; //!< Inputs, inboxes, always have a valid value ( may be NULL )
    tendrils outputs; //!< Outputs, outboxes, always have a valid value ( may be NULL )

  protected:
    virtual void dispatch_declare_params(tendrils& t) = 0;
    virtual void dispatch_declare_io(const tendrils& params, tendrils& inputs,
                                     tendrils& outputs) = 0;
    virtual void dispatch_configure(tendrils& params) = 0;
    virtual ReturnCode
    dispatch_process(const tendrils& inputs, tendrils& outputs) = 0;
    virtual void dispatch_destroy() = 0;
    virtual std::string dispatch_name() const = 0;
    virtual ptr dispatch_make() const
    {
      return ptr();
    }
  };

  /**
   * \brief Helper class for determining if client modules have function
   * implementations or not.
   * @internal
   */
  template<class T>
  struct has_f
  {
    typedef char yes;
    typedef char (&no)[2];

    template<long I> struct S
    {
    };

    // SFINAE eliminates this when the type of arg is invalid
    template<class U>
    static yes test_declare_params(S<sizeof(&U::declare_params)> );
    // overload resolution prefers anything at all over "..."
    template<class U>
    static no test_declare_params(...);

    template<class U>
    static yes test_declare_io(S<sizeof(&U::declare_io)> );
    template<class U>
    static no test_declare_io(...);

    template<class U>
    static yes test_configure(S<sizeof(&U::configure)> );
    template<class U>
    static no test_configure(...);

    template<class U>
    static yes test_process(S<sizeof(&U::process)> );
    template<class U>
    static no test_process(...);

    template<class U>
    static yes test_destroy(S<sizeof(&U::destroy)> );
    template<class U>
    static no test_destroy(...);

    void existent_fn();
    static void existent_static_fn();
    const static S<sizeof(&has_f<T>::existent_fn)> sarg;
    const static S<sizeof(&has_f<T>::existent_static_fn)> ssarg;

    enum
    {
      declare_params = sizeof(test_declare_params<T> (ssarg)) == sizeof(yes)
    };
    enum
    {
      declare_io = sizeof(test_declare_io<T> (ssarg)) == sizeof(yes)
    };
    enum
    {
      configure = sizeof(test_configure<T> (sarg)) == sizeof(yes)
    };
    enum
    {
      process = sizeof(test_process<T> (sarg)) == sizeof(yes)
    };
    enum
    {
      destroy = sizeof(test_destroy<T> (sarg)) == sizeof(yes)
    };

  };

  /**
   * \brief module_<T> is for registering an arbitrary class
   * with the the module NVI. This adds a barrier between client code and the module.
   */
  template<class Module>
  struct module_: module
  {
  protected:
    template<int I>
    struct int_
    {
    };
    typedef int_<0> not_implemented;
    typedef int_<1> implemented;

    static void declare_params(not_implemented, tendrils& params)
    {
    }

    static void declare_params(implemented, tendrils& params)
    {
      Module::declare_params(params);
    }

    void dispatch_declare_params(tendrils& params)
    {
      //this is a none static function. for virtuality.
      declare_params(int_<has_f<Module>::declare_params> (), params);
    }

    static void declare_io(not_implemented, const tendrils& params,
                           tendrils& inputs, tendrils& outputs)
    {
    }
    static void declare_io(implemented, const tendrils& params,
                           tendrils& inputs, tendrils& outputs)
    {
      Module::declare_io(params, inputs, outputs);
    }

    void dispatch_declare_io(const tendrils& params, tendrils& inputs,
                             tendrils& outputs)
    {
      declare_io(int_<has_f<Module>::declare_io> (), params, inputs, outputs);
    }

    void configure(not_implemented, const tendrils& params)
    {
    }

    void configure(implemented, tendrils& params)
    {
      thiz->configure(params);
    }

    void dispatch_configure(tendrils& params)
    {
      //the module may not be allocated here, so check pointer.
      if (!thiz)
        {
          thiz.reset(new Module);
        }
      configure(int_<has_f<Module>::configure> (), params);
    }

    ReturnCode process(not_implemented, const tendrils& inputs,
                       tendrils& outputs)
    {
      return OK;
    }

    ReturnCode process(implemented, const tendrils& inputs, tendrils& outputs)
    {
      return ReturnCode(thiz->process(inputs, outputs));
    }

    ReturnCode dispatch_process(const tendrils& inputs, tendrils& outputs)
    {
      if (!thiz)
        dispatch_configure(parameters);
      return process(int_<has_f<Module>::process> (), inputs, outputs);
    }

    void destroy(not_implemented)
    {
    }

    void destroy(implemented)
    {
      thiz->destroy();
    }

    void dispatch_destroy()
    {
      destroy(int_<has_f<Module>::destroy> ());
    }

    std::string dispatch_name() const
    {
      return MODULE_TYPE_NAME;
    }

    module::ptr dispatch_make() const
    {
      module::ptr m(new module_<Module> ());
      m->declare_params();
      //copy all of the parameters by value.
      tendrils::iterator it = m->parameters.begin();
      tendrils::const_iterator end = m->parameters.end(), oit =
          parameters.begin();
      while (it != end)
        {
          it->second.copy_value(oit->second);
          ++oit;
          ++it;
        }
      m->declare_io();
      return m;
    }
    boost::shared_ptr<Module> thiz;
    static const std::string MODULE_TYPE_NAME;
  };

  template<typename Module>
  const std::string module_<Module>::MODULE_TYPE_NAME = ecto::name_of<Module>();

  /**
   * Creates a module from type T that has not been configured, so therefore,
   * not allocated.  This only calls the static functions associated with parameter and
   * input/output declaration.
   *
   * @return A module::ptr that is initialized as far as default params,inputs,outputs go.
   */
  template<typename T>
  module::ptr inspect_module()
  {
    module::ptr p(new module_<T> ());
    p->declare_params();
    p->declare_io();
    return p;
  }

  /**
   * Create a module from an type that has all of the proper interface functions defined.
   * This will call configure in the module.
   * @return A module ptr.
   */
  template<typename T>
  module::ptr create_module()
  {
    module::ptr p = inspect_module<T> ();
    return p;
  }

}//namespace ecto
