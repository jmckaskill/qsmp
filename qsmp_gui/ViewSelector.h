/******************************************************************************
 * Copyright (C) 2008 James McKaskill <jmckaskill@gmail.com>                  *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License as             *
 * published by the Free Software Foundation; either version 2 of             *
 * the License, or (at your option) any later version.                        *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
 ******************************************************************************/

#ifndef QSMP_VIEWSELECTOR_H_
#define QSMP_VIEWSELECTOR_H_


#include <boost/concept_check.hpp>
#include <boost/function.hpp>
#include <boost/range.hpp>
#include <boost/static_assert.hpp>
#include <qsmp_gui/common.h>
#include <qsmp_gui/utilities.h>
#include <qsmp_gui/TreeModel.h>
#include <qsmp_lib/Log.h>
#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qvariant.h>
#include <QtGui/qtreeview.h>
#include <tcl/sequential_tree.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class ViewTreeModel;

class ViewTreeNode : public TreeModelNode<ViewTreeNode, ViewTreeModel>
{
public:
  ViewTreeNode(ViewTreeModel* model)
    : TreeModelNode<ViewTreeNode, ViewTreeModel>(model)
  {}

  int                 columnCount()const{return 1;}
  QVariant            data(int column, int role)const;

  QModelIndex         AddChild(QString name, shared_ptr<QWidget> widget);

  shared_ptr<QWidget> widget()const{return widget_;}

private:
  QString             name_;
  shared_ptr<QWidget> widget_;
};

//-----------------------------------------------------------------------------

class ViewTreeModel : public TreeModel<ViewTreeNode, ViewTreeModel>
{
  Q_OBJECT
public:
  ViewTreeModel(QLayout* layout)
    : TreeModel<ViewTreeNode, ViewTreeModel>(this),
      layout_(layout),
      old_widget_(NULL)
  {}
  ~ViewTreeModel();

  QModelIndex AddView(QString name, QWidget* widget, const QModelIndex& parent = QModelIndex())
  {return AddView(name, shared_ptr<QWidget>(widget,NullDeleter()), parent);}
  QModelIndex AddNewView(QString name, QWidget* widget, const QModelIndex& parent = QModelIndex())
  {return AddView(name, shared_ptr<QWidget>(widget), parent);}

  QModelIndex AddView(QString name, shared_ptr<QWidget> widget, const QModelIndex& parent = QModelIndex());

public Q_SLOTS:
  void OnClicked(const QModelIndex& index);
private:
  QWidget*  old_widget_;
  QLayout*  layout_;
};

//-----------------------------------------------------------------------------

class ViewTree : public QTreeView
{
public:
  ViewTree(QLayout* layout);

  ViewTreeModel* model(){return &model_;}
private:
  ViewTreeModel model_;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

#endif

