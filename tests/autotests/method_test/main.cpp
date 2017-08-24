#include <QtTest>
#include <boost/hana/replicate.hpp>
#include <reflect.hpp>

using namespace boost::hana::literals;

class MethodTest : public QObject {
  Q_OBJECT
public:
  bool value{true};
  bool value2{false};
  bool bool_method(bool cond = false) { return cond; }
  bool bool_method() { return value; }
  void bool_method(bool &cond) { cond = value; }
  bool &bool_method2() { return value; }
  bool &bool_method3() { return value2; }
  static void static_method(bool cond) { QVERIFY(cond); }
  void const_method(bool cond) const { QVERIFY(cond); }

  IN_METAINFO(MethodTest)
  REFLECT_OBJ_MTD(bool_method, bool)
  REFLECT_OBJ_MTD(bool_method)
  REFLECT_OBJ_MTD(bool_method, bool &)
  REFLECT_OBJ_MTD(bool_method2)
  REFLECT_OBJ_MTD(bool_method3)
  REFLECT_STATIC_MTD(static_method, bool)
  REFLECT_CONST_OBJ_MTD(const_method, bool)

private slots:
  void obj_method();
  void const_method();
  void constexpr_method();
  void static_method();
  void gets_method();
  void sets_method();
  void gets_tuple_args_method();
  void sets_tuple_args_method();
};

class Constexpr_class {
public:
  constexpr auto bool_method() const { return true; }
  auto fail_method() { return false; }
  OUT_METAINFO(Constexpr_class)
};

METAINFO(Constexpr_class)
REFLECT_CONST_OBJ_MTD(bool_method)
REFLECT_OBJ_MTD(fail_method)
END_METAINFO

void MethodTest::obj_method() {
  value = true;
  value2 = false;
  QCOMPARE(typeid(reflect::utils::get<Type, reflect::ObjMethods>(
               0_c, std::ref(*this), true)),
           typeid(true));
  reflect::utils::get<Type, reflect::ObjMethods>(0_c, std::ref(*this), true);
  QCOMPARE(
      (reflect::utils::get<Type, reflect::ObjMethods>(1_c, std::ref(*this))),
      true);
  bool res = false;
  QCOMPARE(typeid(reflect::utils::get<Type, reflect::ObjMethods>(
               2_c, std::ref(*this), res)),
           typeid(boost::hana::type_c<void>));
  reflect::utils::get<Type, reflect::ObjMethods>(2_c, std::ref(*this), res);
  QCOMPARE(res, true);
  QCOMPARE(typeid(reflect::utils::set<Type, reflect::ObjMethods>(
               3_c, true, std::ref(*this))),
           typeid(boost::hana::bool_c<true>));
  reflect::utils::set<Type, reflect::ObjMethods>(3_c, false, std::ref(*this));
  QCOMPARE(value, false);
  QCOMPARE(typeid(reflect::utils::set<Type, reflect::ObjMethods>(
               4_c, true, std::ref(*this))),
           typeid(boost::hana::bool_c<true>));
  reflect::utils::set<Type, reflect::ObjMethods>(4_c, true, std::ref(*this));
  QCOMPARE(value2, true);
}

void MethodTest::const_method() {
  QCOMPARE(typeid(reflect::utils::get<Type, reflect::ConstMethods>(
               0_c, std::ref(*this), true)),
           typeid(boost::hana::type_c<void>));
  reflect::utils::get<Type, reflect::ConstMethods>(0_c, std::ref(*this), true);
}

void MethodTest::constexpr_method() {
  const Constexpr_class obj;
  constexpr bool res =
      reflect::utils::get<Constexpr_class, reflect::ConstMethods>(0_c, obj);
  QCOMPARE(res, true);
}

void MethodTest::static_method() {
  value = true;
  value2 = false;
  QCOMPARE(typeid(reflect::utils::get<Type, reflect::StaticMethods>(
               0_c, std::ref(*this), true)),
           typeid(boost::hana::type_c<void>));
  reflect::utils::get<Type, reflect::StaticMethods>(0_c, std::ref(*this), true);
  QCOMPARE(typeid(reflect::utils::get<Type, reflect::StaticMethods>(0_c, true)),
           typeid(boost::hana::type_c<void>));
  reflect::utils::get<Type, reflect::StaticMethods>(0_c, true);
}

void MethodTest::gets_method() {
  value = true;
  value2 = false;
  auto tup = reflect::utils::gets<Type, reflect::AllMethods>(
      reflect::metautils::gen_inds_tup<decltype(
          reflect::utils::count<Type, reflect::AllMethods>())>(),
      std::ref(*this), true);
  QCOMPARE(tup, boost::hana::make_tuple(
                    true, boost::hana::nothing, boost::hana::nothing,
                    boost::hana::nothing, boost::hana::nothing,
                    boost::hana::type_c<void>, boost::hana::type_c<void>));
}

void MethodTest::sets_method() {
  value = true;
  value2 = false;
  auto tup = reflect::utils::sets<Type, reflect::AllMethods>(
      reflect::metautils::gen_inds_tup<decltype(
          reflect::utils::count<Type, reflect::AllMethods>())>(),
      true, std::ref(*this));
  QCOMPARE(value2, true);
  QCOMPARE(tup,
           boost::hana::make_tuple(
               boost::hana::nothing, boost::hana::nothing, boost::hana::nothing,
               boost::hana::bool_c<true>, boost::hana::bool_c<true>,
               boost::hana::nothing, boost::hana::nothing));
}

void MethodTest::gets_tuple_args_method() {
  bool arg = false;
  value = true;
  value2 = false;
  auto tup = reflect::utils::gets_tuple_args<Type, reflect::AllMethods>(
      reflect::metautils::gen_inds_tup<decltype(
          reflect::utils::count<Type, reflect::AllMethods>())>(),
      boost::hana::make_tuple(boost::hana::make_tuple(std::ref(*this), true),
                              boost::hana::make_tuple(std::ref(*this)),
                              boost::hana::make_tuple(std::ref(*this), arg),
                              boost::hana::make_tuple(std::ref(*this)),
                              boost::hana::make_tuple(std::ref(*this)),
                              boost::hana::make_tuple(true),
                              boost::hana::make_tuple(std::ref(*this), true)));
  QCOMPARE(tup, boost::hana::make_tuple(true, true, boost::hana::type_c<void>,
                                        true, false, boost::hana::type_c<void>,
                                        boost::hana::type_c<void>));
}

void MethodTest::sets_tuple_args_method() {
  value = true;
  value2 = false;
  constexpr auto indices = boost::hana::concat(
      reflect::utils::find_by_name<Type, reflect::AllMethods>("bool_method2"_s),
      reflect::utils::find_by_name<Type, reflect::AllMethods>(
          "bool_method3"_s));
  auto params = boost::hana::replicate<boost::hana::tuple_tag>(
      boost::hana::make_tuple(std::ref(*this)), boost::hana::size(indices));
  constexpr auto values = boost::hana::make_tuple(false, true);
  auto tup = reflect::utils::sets_tuple_args<Type, reflect::AllMethods>(
      indices, values, params);
  QCOMPARE(tup, boost::hana::make_tuple(boost::hana::bool_c<true>,
                                        boost::hana::bool_c<true>));
  QCOMPARE(value, false);
  QCOMPARE(value2, true);
}

QTEST_MAIN(MethodTest)
#include "main.moc"
