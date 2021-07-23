// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <cc/user_context.h>
#include <gtest/gtest.h>

using namespace isc::data;

namespace {

ElementPtr gen() {
    std::string content = "{ \"foo\": 1, \"bar\": \"xyz\" }";
    return (Element::fromJSON(content));
}

TEST(UserContext, setget) {
    UserContext parent;
    EXPECT_FALSE(parent.getContext());
    ElementPtr map = gen();
    parent.setContext(map);
    ElementPtr ctx = parent.getContext();
    EXPECT_EQ(*ctx, *map);
}

TEST(UserContext, null) {
    UserContext parent;
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    EXPECT_EQ(*expected, *map);
}

TEST(UserContext, notMap) {
    UserContext parent;
    ElementPtr ctx = Element::create("foo");
    parent.setContext(ctx);
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    expected->set("user-context", ctx);
    EXPECT_EQ(*expected, *map);
}

TEST(UserContext, empty) {
    UserContext parent;
    ElementPtr ctx = Element::createMap();
    parent.setContext(ctx);
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    expected->set("user-context", ctx);
    EXPECT_EQ(*expected, *map);
}

TEST(UserContext, noComment) {
    UserContext parent;
    ElementPtr ctx = Element::fromJSON("{ \"version\": 1 }");
    parent.setContext(ctx);
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    expected->set("user-context", ctx);
    EXPECT_EQ(*expected, *map);
};

TEST(UserContext, onlyComment) {
    UserContext parent;
    ElementPtr ctx = Element::fromJSON("{ \"comment\": \"foobar\" }");
    parent.setContext(ctx);
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    expected->set("user-context", ctx);
    EXPECT_EQ(*expected, *map);
}

TEST(UserContext, both) {
    UserContext parent;
    ElementPtr ctx =
        Element::fromJSON("{ \"comment\": \"foobar\", \"version\": 1 }");
    parent.setContext(ctx);
    ElementPtr map = gen();
    parent.contextToElement(map);
    ElementPtr expected = gen();
    expected->set("user-context", ctx);
    EXPECT_EQ(*expected, *map);
}

TEST(toElement, notMap) {
    ElementPtr arg = Element::create("foo");
    ElementPtr result = UserContext::toElement(arg);
    EXPECT_EQ(*result, *arg);
}

TEST(toElement, empty) {
    ElementPtr map = gen();
    ElementPtr ctx = Element::createMap();
    map->set("user-context", ctx);
    ElementPtr result = UserContext::toElement(map);
    EXPECT_EQ(*result, *map);
}

TEST(toElement, noComment) {
    ElementPtr map = gen();
    ElementPtr ctx = Element::fromJSON("{ \"version\": 1 }");
    map->set("user-context", ctx);
    ElementPtr result = UserContext::toElement(map);
    EXPECT_EQ(*result, *map);
}

TEST(toElement, onlyComment) {
    ElementPtr map = gen();
    ElementPtr ctx = Element::fromJSON("{ \"comment\": \"foobar\" }");
    map->set("user-context", ctx);
    ElementPtr result = UserContext::toElement(map);
    EXPECT_EQ(*result, *map);
}

TEST(toElement, both) {
    ElementPtr map = gen();
    ElementPtr ctx =
        Element::fromJSON("{ \"comment\": \"foobar\", \"version\": 1 }");
    map->set("user-context", ctx);
    ElementPtr result = UserContext::toElement(map);
    EXPECT_EQ(*result, *map);
}

}  // namespace
