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

QSMP_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ViewSelector::ViewSelector(QWidget* parent_widget)
: parent_widget_(parent_widget)
{
  model_ = new Model(boost::bind(&ViewSelector::tree,this));
  model_->set_clicked(boost::bind(&ViewSelector::ActivateEntry,this,_1));
  model_->SetView(this);
  setModel(model_);
}

//-----------------------------------------------------------------------------

ViewSelectorNode* ViewSelector::AddViewEntry(boost::function<QLayout* ()> new_view,
                                             QString                      text,
                                             ViewSelectorNode*            parent)
{
  if (!parent)
    parent = &tree_;

  ViewEntry view;
  view.new_view_ = new_view;
  view.text_     = text;
  parent->push_back(view);
  model_->reset();
  return &*parent->rbegin();
}

//-----------------------------------------------------------------------------

void ViewSelector::ActivateEntry(const ViewSelectorNode& node)
{
  delete parent_widget_->layout();
  parent_widget_->setLayout(node.get().new_view_());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

QSMP_END