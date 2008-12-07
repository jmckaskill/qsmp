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

#include "stdafx.h"

#include <boost/bind.hpp>
#include <qsmp_gui/ViewSelector.h>
#include <qsmp_gui/ViewSelector.moc>
#include <QtGui/qlayout.h>

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QVariant ViewTreeNode::data(int column, int role)const
{
  if (column == 0 && role == Qt::DisplayRole)
  {
    return name_;
  }
  return QVariant();
}

//-----------------------------------------------------------------------------

QModelIndex ViewTreeNode::AddChild(QString name, shared_ptr<QWidget> widget)
{
  BeginAddChildren(1);

  ViewTreeModel* m = model();

  ViewTreeNode* child = AddChildren(1);

  child->name_   = name;
  child->widget_ = widget;

  EndAddChildren(m);
  return child->index(0);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ViewTreeModel::~ViewTreeModel()
{
  if (old_widget_)
    layout_->removeWidget(old_widget_);
}

//-----------------------------------------------------------------------------

QModelIndex ViewTreeModel::AddView(QString name, shared_ptr<QWidget> widget, const QModelIndex& parent)
{
  return FromIndex(parent)->AddChild(name, widget);
}

//-----------------------------------------------------------------------------

void ViewTreeModel::OnClicked(const QModelIndex& index)
{
  if (old_widget_)
  {
    layout_->removeWidget(old_widget_);
    old_widget_->hide();
  }
  old_widget_ = FromIndex(index)->widget().get();
  layout_->addWidget(old_widget_);
  old_widget_->show();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ViewTree::ViewTree(QLayout* layout)
: model_(layout)
{
  setModel(model());
  connect(this, SIGNAL(clicked(const QModelIndex&)), &model_, SLOT(OnClicked(const QModelIndex&)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END

