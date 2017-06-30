// Copyright 2017 COINSE Lab.
#include <iostream>
#include <sstream>

#include "Consumers.h"

bool MyASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef DR) {
  for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
       ++b) {
    if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
      if (f->getName() == target) {
        // Traverse the declaration using our AST visitor.
        Visitor.TraverseDecl(*b);
      }
    }
  }

  return true;
}

bool DeclarationConsumer::HandleTopLevelDecl(clang::DeclGroupRef DR) {
  for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
       ++b) {
    if (clang::NamedDecl *d = clang::dyn_cast<clang::NamedDecl>(*b)) {
      if (d->getName() == target) {
        if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
          if (f->hasBody()) {
            std::stringstream ss;
            ss << clang::QualType::getAsString(
                      f->getReturnType().getCanonicalType().split())
               << ' ';
            ss << f->getNameAsString();
            ss << '(';
            int i = 0;
            for (auto &it : f->parameters()) {
              if (i > 0)
                ss << ',';
              std::string t = clang::QualType::getAsString(
                  it->getType().getCanonicalType().split());
              std::get<1>(decl).push_back(t);
              ss << t << ' ';
              ss << it->getNameAsString();

              i++;
            }
            ss << ");";
            std::get<0>(decl) = ss.str();
          }
        } else if (clang::RecordDecl *f =
                       clang::dyn_cast<clang::RecordDecl>(*b)) {
          std::stringstream ss;
          ss << "struct ";
          ss << f->getNameAsString();
          ss << "{";
          for (const auto &it : f->fields()) {
            std::string t = clang::QualType::getAsString(
                it->getType().getCanonicalType().split());
            std::get<1>(decl).push_back(t);
            ss << t << ' ';
            ss << it->getNameAsString();
            ss << "; ";
          }
          ss << "};";
          std::get<0>(decl) = ss.str();
        }
      }
    }
  }
  return true;
}

bool FunctionConsumer::HandleTopLevelDecl(clang::DeclGroupRef DR) {
  for (clang::DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e;
       ++b) {
    if (clang::FunctionDecl *f = clang::dyn_cast<clang::FunctionDecl>(*b)) {
      if (f->hasBody()) {
        decls.push_back(f->getNameAsString());
        std::stringstream ss;
        ss << clang::QualType::getAsString(
                  f->getReturnType().getCanonicalType().split())
           << ' ';
        ss << f->getNameAsString();
        ss << '(';
        int i = 0;
        for (auto &it : f->parameters()) {
          if (i > 0)
            ss << ", ";
          std::string t = clang::QualType::getAsString(
              it->getType().getCanonicalType().split());
          ss << t << ' ';
          ss << it->getNameAsString();

          i++;
        }
        ss << ");";
        std::cout << "\t" << ss.str() << std::endl;
      }
    }
  }
  return true;
}

std::vector<std::string> FunctionConsumer::getFunctions() { return decls; }
