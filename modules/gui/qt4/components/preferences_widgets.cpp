/*****************************************************************************
 * preferences_widgets.cpp : Widgets for preferences displays
 ****************************************************************************
 * Copyright (C) 2006-2007 the VideoLAN team
 * $Id$
 *
 * Authors: Clément Stenac <zorglub@videolan.org>
 *          Antoine Cellerier <dionoea@videolan.org>
 *          Jean-Baptiste Kempf <jb@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/**
 * Todo:
 *  - Finish implementation (see WX)
 *  - Improvements over WX
 *      - Validator for modulelist
 *  - Implement update stuff using a general Updated signal
 */

#include "components/preferences_widgets.hpp"
#include "util/customwidgets.hpp"

#include <vlc_keys.h>

#include <QString>
#include <QVariant>
#include <QGridLayout>
#include <QSlider>
#include <QFileDialog>
#include <QFontDialog>
#include <QGroupBox>
#include <QTreeWidgetItem>
#include <QSignalMapper>
QString formatTooltip(const QString & tooltip)
{
    QString formatted =
    "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">"
    " p, li { white-space: pre-wrap; } </style></head><body style=\" font-family:'Sans Serif';"
    " font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\">"
    "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
    "-qt-block-indent:0; text-indent:0px;\">" +
    tooltip +
    "</p></body></html>";
    return formatted;
}

ConfigControl *ConfigControl::createControl( vlc_object_t *p_this,
                                             module_config_t *p_item,
                                             QWidget *parent )
{
    int i=0;
    return createControl( p_this, p_item, parent, NULL, i );
}

ConfigControl *ConfigControl::createControl( vlc_object_t *p_this,
                                             module_config_t *p_item,
                                             QWidget *parent,
                                             QGridLayout *l, int &line )
{
    ConfigControl *p_control = NULL;
    if( p_item->psz_current || p_item->b_unsaveable ) return NULL;

    switch( p_item->i_type )
    {
    case CONFIG_ITEM_MODULE:
        p_control = new ModuleConfigControl( p_this, p_item, parent, false,
                                             l, line );
        break;
    case CONFIG_ITEM_MODULE_CAT:
        p_control = new ModuleConfigControl( p_this, p_item, parent, true,
                                             l, line );
        break;
    case CONFIG_ITEM_MODULE_LIST:
        p_control = new ModuleListConfigControl( p_this, p_item, parent, false,
                                             l, line );
        break;
    case CONFIG_ITEM_MODULE_LIST_CAT:
        p_control = new ModuleListConfigControl( p_this, p_item, parent, true,
                                             l, line );
        break;
    case CONFIG_ITEM_STRING:
        if( !p_item->i_list )
            p_control = new StringConfigControl( p_this, p_item, parent,
                                                 l, line, false );
        else
            p_control = new StringListConfigControl( p_this, p_item,
                                            parent, false, l, line );
        break;
    case CONFIG_ITEM_PASSWORD:
        if( !p_item->i_list )
            p_control = new StringConfigControl( p_this, p_item, parent,
                                                 l, line, true );
        else
            p_control = new StringListConfigControl( p_this, p_item,
                                            parent, true, l, line );
        break;
    case CONFIG_ITEM_INTEGER:
        if( p_item->i_list )
            p_control = new IntegerListConfigControl( p_this, p_item,
                                            parent, false, l, line );
        else if( p_item->min.i || p_item->max.i )
            p_control = new IntegerRangeConfigControl( p_this, p_item, parent,
                                                       l, line );
        else
            p_control = new IntegerConfigControl( p_this, p_item, parent,
                                                  l, line );
        break;
    case CONFIG_ITEM_FILE:
        p_control = new FileConfigControl( p_this, p_item, parent, l,
                                                line, false );
        break;
    case CONFIG_ITEM_DIRECTORY:
        p_control = new DirectoryConfigControl( p_this, p_item, parent, l,
                                                line, false );
        break;
    case CONFIG_ITEM_FONT:
        p_control = new FontConfigControl( p_this, p_item, parent, l,
                                           line, false );
        break;
    case CONFIG_ITEM_KEY:
        p_control = new KeySelectorControl( p_this, p_item, parent, l, line );
        break;
    case CONFIG_ITEM_BOOL:
        p_control = new BoolConfigControl( p_this, p_item, parent, l, line );
        break;
    case CONFIG_ITEM_FLOAT:
        if( p_item->min.f || p_item->max.f )
            p_control = new FloatRangeConfigControl( p_this, p_item, parent,
                                                     l, line );
        else
            p_control = new FloatConfigControl( p_this, p_item, parent,
                                                  l, line );
        break;
    default:
        break;
    }
    return p_control;
}

void ConfigControl::doApply( intf_thread_t *p_intf )
{
    switch( getType() )
    {
        case 1:
        {
            VIntConfigControl *vicc = qobject_cast<VIntConfigControl *>(this);
            assert( vicc );
            config_PutInt( p_intf, vicc->getName(), vicc->getValue() );
            break;
        }
        case 2:
        {
            VFloatConfigControl *vfcc =
                                    qobject_cast<VFloatConfigControl *>(this);
            assert( vfcc );
            config_PutFloat( p_intf, vfcc->getName(), vfcc->getValue() );
            break;
        }
        case 3:
        {
            VStringConfigControl *vscc =
                            qobject_cast<VStringConfigControl *>(this);
            assert( vscc );
            config_PutPsz( p_intf, vscc->getName(), qta( vscc->getValue() ) );
            break;
        }
        case 4:
        {
            KeySelectorControl *ksc = qobject_cast<KeySelectorControl *>(this);
            assert( ksc );
            ksc->doApply();
        }
    }
}

/**************************************************************************
 * String-based controls
 *************************************************************************/

/*********** String **************/
StringConfigControl::StringConfigControl( vlc_object_t *_p_this,
                                          module_config_t *_p_item,
                                          QWidget *_parent, QGridLayout *l,
                                          int &line, bool pwd ) :
                           VStringConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    text = new QLineEdit( qfu(p_item->value.psz) );
    if( pwd ) text->setEchoMode( QLineEdit::Password );
    finish();

    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label, 0 ); layout->addWidget( text, 1 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 ); l->addWidget( text, line, 1 );
    }
}

StringConfigControl::StringConfigControl( vlc_object_t *_p_this,
                                   module_config_t *_p_item,
                                   QLabel *_label, QLineEdit *_text, bool pwd ):
                           VStringConfigControl( _p_this, _p_item )
{
    text = _text;
    label = _label;
    finish( );
}

void StringConfigControl::finish()
{
    text->setText( qfu(p_item->value.psz) );
    text->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

/*********** File **************/
FileConfigControl::FileConfigControl( vlc_object_t *_p_this,
                                          module_config_t *_p_item,
                                          QWidget *_parent, QGridLayout *l,
                                          int &line, bool pwd ) :
                           VStringConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    text = new QLineEdit( qfu(p_item->value.psz) );
    browse = new QPushButton( qtr( "Browse..." ) );
    QHBoxLayout *textAndButton = new QHBoxLayout();
    textAndButton->setMargin( 0 );
    textAndButton->addWidget( text, 2 );
    textAndButton->addWidget( browse, 0 );

    BUTTONACT( browse, updateField() );

    finish();

    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label, 0 );
        layout->addLayout( textAndButton, 1 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addLayout( textAndButton, line, 1 );
    }
}


FileConfigControl::FileConfigControl( vlc_object_t *_p_this,
                                   module_config_t *_p_item,
                                   QLabel *_label, QLineEdit *_text,
                                   QPushButton *_button, bool pwd ):
                           VStringConfigControl( _p_this, _p_item )
{
    browse = _button;
    text = _text;
    label = _label;

    BUTTONACT( browse, updateField() );

    finish( );
}

void FileConfigControl::updateField()
{
    QString file = QFileDialog::getOpenFileName( NULL,
                  qtr( "Select File" ), qfu( p_this->p_libvlc->psz_homedir ) );
    if( file.isNull() ) return;
    text->setText( file );
}

void FileConfigControl::finish()
{
    text->setText( qfu(p_item->value.psz) );
    text->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

/********* String / Directory **********/
DirectoryConfigControl::DirectoryConfigControl( vlc_object_t *_p_this,
                        module_config_t *_p_item, QWidget *_p_widget,
                        QGridLayout *_p_layout, int& _int, bool _pwd ) :
     FileConfigControl( _p_this, _p_item, _p_widget, _p_layout, _int, _pwd)
{}

DirectoryConfigControl::DirectoryConfigControl( vlc_object_t *_p_this,
                        module_config_t *_p_item, QLabel *_p_label,
                        QLineEdit *_p_line, QPushButton *_p_button, bool _pwd ):
     FileConfigControl( _p_this, _p_item, _p_label, _p_line, _p_button, _pwd)
{}

void DirectoryConfigControl::updateField()
{
    QString dir = QFileDialog::getExistingDirectory( NULL,
                      qtr( "Select Directory" ),
                      text->text().isEmpty() ?
                        qfu( p_this->p_libvlc->psz_homedir ) : text->text(),
                      QFileDialog::ShowDirsOnly |
                        QFileDialog::DontResolveSymlinks );
    if( dir.isNull() ) return;
    text->setText( dir );
}

/********* String / Font **********/
FontConfigControl::FontConfigControl( vlc_object_t *_p_this,
                        module_config_t *_p_item, QWidget *_p_widget,
                        QGridLayout *_p_layout, int& _int, bool _pwd ) :
     FileConfigControl( _p_this, _p_item, _p_widget, _p_layout, _int, _pwd)
{}

FontConfigControl::FontConfigControl( vlc_object_t *_p_this,
                        module_config_t *_p_item, QLabel *_p_label,
                        QLineEdit *_p_line, QPushButton *_p_button, bool _pwd ):
     FileConfigControl( _p_this, _p_item, _p_label, _p_line, _p_button, _pwd)
{}

void FontConfigControl::updateField()
{
    bool ok;
    QFont font = QFontDialog::getFont( &ok, QFont( text->text() ), NULL );
    if( !ok ) return;
    text->setText( font.family() );
}

/********* String / choice list **********/
StringListConfigControl::StringListConfigControl( vlc_object_t *_p_this,
               module_config_t *_p_item, QWidget *_parent, bool bycat,
               QGridLayout *l, int &line) :
               VStringConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    combo = new QComboBox();
    finish( bycat );
    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label ); layout->addWidget( combo );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addWidget( combo, line, 1, Qt::AlignRight );
    }

    if( p_item->i_action )
    {
        QSignalMapper *signalMapper = new QSignalMapper(this);

        /* Some stringLists like Capture listings have action associated */
        for( int i = 0; i < p_item->i_action; i++ )
        {
            QPushButton *button =
                new QPushButton( qfu( p_item->ppsz_action_text[i] ));
            CONNECT( button, clicked(), signalMapper, map() );
            signalMapper->setMapping( button, i );
            l->addWidget( button, line, 2 + i, Qt::AlignRight );
        }
        CONNECT( signalMapper, mapped( int ),
                this, actionRequested( int ) );
    }
}

void StringListConfigControl::actionRequested( int i_action )
{
    /* Supplementary check for boundaries */
    if( i_action < 0 || i_action >= p_item->i_action ) return;

    vlc_value_t val;
    val.psz_string = qtu( (combo->itemData( combo->currentIndex() ).toString() ) );

    p_item->ppf_action[i_action]( p_this, getName(), val, val, 0 );

    if( p_item->b_dirty )
    {
        combo->clear();
        finish( true );
        p_item->b_dirty = VLC_FALSE;

    }
}
StringListConfigControl::StringListConfigControl( vlc_object_t *_p_this,
                module_config_t *_p_item, QLabel *_label, QComboBox *_combo,
                bool bycat ) : VStringConfigControl( _p_this, _p_item )
{
    combo = _combo;
    label = _label;
    finish( bycat );
}

void StringListConfigControl::finish( bool bycat )
{
    combo->setEditable( false );

    for( int i_index = 0; i_index < p_item->i_list; i_index++ )
    {
        combo->addItem( qfu(p_item->ppsz_list_text ?
                            p_item->ppsz_list_text[i_index] :
                            p_item->ppsz_list[i_index] ),
                        QVariant( p_item->ppsz_list[i_index] ) );
        if( p_item->value.psz && !strcmp( p_item->value.psz,
                                          p_item->ppsz_list[i_index] ) )
            combo->setCurrentIndex( combo->count() - 1 );
    }
    combo->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

QString StringListConfigControl::getValue()
{
    return combo->itemData( combo->currentIndex() ).toString();
}

void setfillVLCConfigCombo( const char *configname, intf_thread_t *p_intf,
                        QComboBox *combo, QWidget *parent )
{
    module_config_t *p_config =
                      config_FindConfig( VLC_OBJECT(p_intf), configname );
    if( p_config )
    {
        for ( int i_index = 0; i_index < p_config->i_list; i_index++ )
        {
            combo->addItem( qfu( p_config->ppsz_list_text[i_index] ),
                    QVariant( p_config->pi_list[i_index] ) );
            if( p_config->value.i == p_config->pi_list[i_index] )
            {
                combo->setCurrentIndex( i_index );
            }
        }
        combo->setToolTip( qfu( p_config->psz_longtext ) );
    }
}

/********* Module **********/
ModuleConfigControl::ModuleConfigControl( vlc_object_t *_p_this,
               module_config_t *_p_item, QWidget *_parent, bool bycat,
               QGridLayout *l, int &line) :
               VStringConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    combo = new QComboBox();
    finish( bycat );
    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label ); layout->addWidget( combo );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addWidget( combo, line, 1, Qt::AlignRight );
    }
}

ModuleConfigControl::ModuleConfigControl( vlc_object_t *_p_this,
                module_config_t *_p_item, QLabel *_label, QComboBox *_combo,
                bool bycat ) : VStringConfigControl( _p_this, _p_item )
{
    combo = _combo;
    label = _label;
    finish( bycat );
}

void ModuleConfigControl::finish( bool bycat )
{
    vlc_list_t *p_list;
    module_t *p_parser;

    combo->setEditable( false );

    /* build a list of available modules */
    p_list = vlc_list_find( p_this, VLC_OBJECT_MODULE, FIND_ANYWHERE );
    combo->addItem( qtr("Default") );
    for( int i_index = 0; i_index < p_list->i_count; i_index++ )
    {
        p_parser = (module_t *)p_list->p_values[i_index].p_object ;

        if( bycat )
        {
            if( !strcmp( module_GetObjName( p_parser ), "main" ) ) continue;

            for (size_t i = 0; i < p_parser->confsize; i++)
            {
                module_config_t *p_config = p_parser->p_config + i;
                /* Hack: required subcategory is stored in i_min */
                if( p_config->i_type == CONFIG_SUBCATEGORY &&
                    p_config->value.i == p_item->min.i )
                    combo->addItem( qtr( module_GetLongName( p_parser )),
                                    QVariant( module_GetObjName( p_parser ) ) );
                if( p_item->value.psz && !strcmp( p_item->value.psz,
                                                  module_GetObjName( p_parser ) ) )
                    combo->setCurrentIndex( combo->count() - 1 );
            }
        }
        else if( module_IsCapable( p_parser, p_item->psz_type ) )
        {
            combo->addItem( qtr(module_GetLongName( p_parser ) ),
                            QVariant( module_GetObjName( p_parser ) ) );
            if( p_item->value.psz && !strcmp( p_item->value.psz,
                                              module_GetObjName( p_parser ) ) )
                combo->setCurrentIndex( combo->count() - 1 );
        }
    }
    vlc_list_release( p_list );
    combo->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

QString ModuleConfigControl::getValue()
{
    return combo->itemData( combo->currentIndex() ).toString();
}

/********* Module list **********/
ModuleListConfigControl::ModuleListConfigControl( vlc_object_t *_p_this,
        module_config_t *_p_item, QWidget *_parent, bool bycat,
        QGridLayout *l, int &line) :
    VStringConfigControl( _p_this, _p_item, _parent )
{
    groupBox = new QGroupBox ( qtr(p_item->psz_text) );
    text = new QLineEdit();
    QGridLayout *layoutGroupBox = new QGridLayout( groupBox );

    finish( bycat );

    int boxline = 0;
    for( QVector<checkBoxListItem*>::iterator it = modules.begin();
            it != modules.end(); it++ )
    {
        layoutGroupBox->addWidget( (*it)->checkBox, boxline++, 0 );
    }
    layoutGroupBox->addWidget( text, boxline, 0 );

    if( !l )
    {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget( groupBox, line, 0 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( groupBox, line, 0, 1, -1 );
    }

    text->setToolTip( formatTooltip( qtr( p_item->psz_longtext) ) );
}

ModuleListConfigControl::~ModuleListConfigControl()
{
    for( QVector<checkBoxListItem*>::iterator it = modules.begin();
            it != modules.end(); it++ )
    {
        delete *it;
    }
    delete groupBox;
    delete text;
}

#define CHECKBOX_LISTS \
{ \
       QCheckBox *cb = new QCheckBox( qtr( module_GetLongName( p_parser ) ) );\
       checkBoxListItem *cbl = new checkBoxListItem; \
\
       CONNECT( cb, stateChanged( int ), this, onUpdate( int ) );\
       cb->setToolTip( formatTooltip( qtr( module_GetLongName( p_parser ))));\
       cbl->checkBox = cb; \
\
       int i = -1; \
       while( p_parser->pp_shortcuts[++i] != NULL); \
       i--; \
\
       cbl->psz_module = strdup( i>=0?p_parser->pp_shortcuts[i] \
                                 : module_GetObjName( p_parser ) ); \
       modules.push_back( cbl ); \
}


void ModuleListConfigControl::finish( bool bycat )
{
    vlc_list_t *p_list;
    module_t *p_parser;

    /* build a list of available modules */
    p_list = vlc_list_find( p_this, VLC_OBJECT_MODULE, FIND_ANYWHERE );
    for( int i_index = 0; i_index < p_list->i_count; i_index++ )
    {
        p_parser = (module_t *)p_list->p_values[i_index].p_object ;

        if( bycat )
        {
            if( !strcmp( module_GetObjName( p_parser ), "main" ) ) continue;

            for (size_t i = 0; i < p_parser->confsize; i++)
            {
                module_config_t *p_config = p_parser->p_config + i;
                /* Hack: required subcategory is stored in i_min */
                if( p_config->i_type == CONFIG_SUBCATEGORY &&
                        p_config->value.i == p_item->min.i )
                {
                    CHECKBOX_LISTS;
                }
            }
        }
        else if( module_IsCapable( p_parser, p_item->psz_type ) )
        {
            CHECKBOX_LISTS;
        }
    }
    vlc_list_release( p_list );
    text->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( groupBox )
        groupBox->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}
#undef CHECKBOX_LISTS

QString ModuleListConfigControl::getValue()
{
    return text->text();
}

void ModuleListConfigControl::hide()
{
    for( QVector<checkBoxListItem*>::iterator it = modules.begin();
         it != modules.end(); it++ )
    {
        (*it)->checkBox->hide();
    }
    groupBox->hide();
}

void ModuleListConfigControl::show()
{
    for( QVector<checkBoxListItem*>::iterator it = modules.begin();
         it != modules.end(); it++ )
    {
        (*it)->checkBox->show();
    }
    groupBox->show();
}


void ModuleListConfigControl::onUpdate( int value )
{
    text->clear();
    bool first = true;

    for( QVector<checkBoxListItem*>::iterator it = modules.begin();
         it != modules.end(); it++ )
    {
        if( (*it)->checkBox->isChecked() )
        {
            if( first )
            {
                text->setText( text->text() + (*it)->psz_module );
                first = false;
            }
            else
            {
                text->setText( text->text() + ":" + (*it)->psz_module );
            }
        }
    }
    emit Updated();
}

/**************************************************************************
 * Integer-based controls
 *************************************************************************/

/*********** Integer **************/
IntegerConfigControl::IntegerConfigControl( vlc_object_t *_p_this,
                                            module_config_t *_p_item,
                                            QWidget *_parent, QGridLayout *l,
                                            int &line ) :
                           VIntConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    spin = new QSpinBox; spin->setMinimumWidth( 80 );
    spin->setAlignment( Qt::AlignRight );
    spin->setMaximumWidth( 90 );
    finish();

    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label, 0 ); layout->addWidget( spin, 1 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addWidget( spin, line, 1, Qt::AlignRight );
    }
}
IntegerConfigControl::IntegerConfigControl( vlc_object_t *_p_this,
                                            module_config_t *_p_item,
                                            QLabel *_label, QSpinBox *_spin ) :
                                      VIntConfigControl( _p_this, _p_item )
{
    spin = _spin;
    label = _label;
    finish();
}

void IntegerConfigControl::finish()
{
    spin->setMaximum( 2000000000 );
    spin->setMinimum( -2000000000 );
    spin->setValue( p_item->value.i );
    spin->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

int IntegerConfigControl::getValue()
{
    return spin->value();
}

/********* Integer range **********/
IntegerRangeConfigControl::IntegerRangeConfigControl( vlc_object_t *_p_this,
                                            module_config_t *_p_item,
                                            QWidget *_parent, QGridLayout *l,
                                            int &line ) :
            IntegerConfigControl( _p_this, _p_item, _parent, l, line )
{
    finish();
}

IntegerRangeConfigControl::IntegerRangeConfigControl( vlc_object_t *_p_this,
                                            module_config_t *_p_item,
                                            QLabel *_label, QSpinBox *_spin ) :
            IntegerConfigControl( _p_this, _p_item, _label, _spin )
{
    finish();
}

void IntegerRangeConfigControl::finish()
{
    spin->setMaximum( p_item->max.i );
    spin->setMinimum( p_item->min.i );
}

IntegerRangeSliderConfigControl::IntegerRangeSliderConfigControl(
                                            vlc_object_t *_p_this,
                                            module_config_t *_p_item,
                                            QLabel *_label, QSlider *_slider ):
                    VIntConfigControl( _p_this, _p_item )
{
    slider = _slider;
    label = _label;
    slider->setMaximum( p_item->max.i );
    slider->setMinimum( p_item->min.i );
    slider->setValue( p_item->value.i );
    slider->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

int IntegerRangeSliderConfigControl::getValue()
{
        return slider->value();
}


/********* Integer / choice list **********/
IntegerListConfigControl::IntegerListConfigControl( vlc_object_t *_p_this,
               module_config_t *_p_item, QWidget *_parent, bool bycat,
               QGridLayout *l, int &line) :
               VIntConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    combo = new QComboBox();
    finish( bycat );
    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label ); layout->addWidget( combo );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addWidget( combo, line, 1, Qt::AlignRight );
    }
}
IntegerListConfigControl::IntegerListConfigControl( vlc_object_t *_p_this,
                module_config_t *_p_item, QLabel *_label, QComboBox *_combo,
                bool bycat ) : VIntConfigControl( _p_this, _p_item )
{
    combo = _combo;
    label = _label;
    finish( bycat );
}

void IntegerListConfigControl::finish( bool bycat )
{
    combo->setEditable( false );

    for( int i_index = 0; i_index < p_item->i_list; i_index++ )
    {
        combo->addItem( qtr(p_item->ppsz_list_text[i_index] ),
                        QVariant( p_item->pi_list[i_index] ) );
        if( p_item->value.i == p_item->pi_list[i_index] )
            combo->setCurrentIndex( combo->count() - 1 );
    }
    combo->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

int IntegerListConfigControl::getValue()
{
    return combo->itemData( combo->currentIndex() ).toInt();
}

/*********** Boolean **************/
BoolConfigControl::BoolConfigControl( vlc_object_t *_p_this,
                                      module_config_t *_p_item,
                                      QWidget *_parent, QGridLayout *l,
                                      int &line ) :
                    VIntConfigControl( _p_this, _p_item, _parent )
{
    checkbox = new QCheckBox( qtr(p_item->psz_text) );
    finish();

    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( checkbox, 0 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( checkbox, line, 0 );
    }
}
BoolConfigControl::BoolConfigControl( vlc_object_t *_p_this,
                                      module_config_t *_p_item,
                                      QLabel *_label,
                                      QCheckBox *_checkbox,
                                      bool bycat ) :
                   VIntConfigControl( _p_this, _p_item )
{
    checkbox = _checkbox;
    finish();
}

void BoolConfigControl::finish()
{
    checkbox->setCheckState( p_item->value.i == VLC_TRUE ? Qt::Checked
                                                        : Qt::Unchecked );
    checkbox->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

int BoolConfigControl::getValue()
{
    return checkbox->checkState() == Qt::Checked ? VLC_TRUE : VLC_FALSE;
}

/**************************************************************************
 * Float-based controls
 *************************************************************************/

/*********** Float **************/
FloatConfigControl::FloatConfigControl( vlc_object_t *_p_this,
                                        module_config_t *_p_item,
                                        QWidget *_parent, QGridLayout *l,
                                        int &line ) :
                    VFloatConfigControl( _p_this, _p_item, _parent )
{
    label = new QLabel( qtr(p_item->psz_text) );
    spin = new QDoubleSpinBox; spin->setMinimumWidth( 80 );
    spin->setMaximumWidth( 90 );
    spin->setAlignment( Qt::AlignRight );
    finish();

    if( !l )
    {
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addWidget( label, 0 ); layout->addWidget( spin, 1 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0 );
        l->addWidget( spin, line, 1, Qt::AlignRight );
    }
}

FloatConfigControl::FloatConfigControl( vlc_object_t *_p_this,
                                        module_config_t *_p_item,
                                        QLabel *_label,
                                        QDoubleSpinBox *_spin ) :
                    VFloatConfigControl( _p_this, _p_item )
{
    spin = _spin;
    label = _label;
    finish();
}

void FloatConfigControl::finish()
{
    spin->setMaximum( 2000000000. );
    spin->setMinimum( -2000000000. );
    spin->setSingleStep( 0.1 );
    spin->setValue( (double)p_item->value.f );
    spin->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );
}

float FloatConfigControl::getValue()
{
    return (float)spin->value();
}

/*********** Float with range **************/
FloatRangeConfigControl::FloatRangeConfigControl( vlc_object_t *_p_this,
                                        module_config_t *_p_item,
                                        QWidget *_parent, QGridLayout *l,
                                        int &line ) :
                FloatConfigControl( _p_this, _p_item, _parent, l, line )
{
    finish();
}

FloatRangeConfigControl::FloatRangeConfigControl( vlc_object_t *_p_this,
                                        module_config_t *_p_item,
                                        QLabel *_label,
                                        QDoubleSpinBox *_spin ) :
                FloatConfigControl( _p_this, _p_item, _label, _spin )
{
    finish();
}

void FloatRangeConfigControl::finish()
{
    spin->setMaximum( (double)p_item->max.f );
    spin->setMinimum( (double)p_item->min.f );
}


/**********************************************************************
 * Key selector widget
 **********************************************************************/
KeySelectorControl::KeySelectorControl( vlc_object_t *_p_this,
                                      module_config_t *_p_item,
                                      QWidget *_parent, QGridLayout *l,
                                      int &line ) :
                                ConfigControl( _p_this, _p_item, _parent )

{
    label = new QLabel( qtr("Select an action to change the associated hotkey") );
    table = new QTreeWidget( 0 );
    finish();

    if( !l )
    {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget( label, 0 ); layout->addWidget( table, 1 );
        widget->setLayout( layout );
    }
    else
    {
        l->addWidget( label, line, 0, 1, 2 );
        l->addWidget( table, line+1, 0, 1,2 );
    }
}

void KeySelectorControl::finish()
{
    if( label )
        label->setToolTip( formatTooltip(qtr(p_item->psz_longtext)) );

    /* Fill the table */
    table->setColumnCount( 2 );
    table->setAlternatingRowColors( true );

    module_t *p_main = config_FindModule( p_this, "main" );
    assert( p_main );

    for (size_t i = 0; i < p_main->confsize; i++)
    {
        module_config_t *p_item = p_main->p_config + i;

        if( p_item->i_type & CONFIG_ITEM && p_item->psz_name &&
            strstr( p_item->psz_name , "key-" ) )
        {
            QTreeWidgetItem *treeItem = new QTreeWidgetItem();
            treeItem->setText( 0, qtr( p_item->psz_text ) );
            treeItem->setText( 1, VLCKeyToString( p_item->value.i ) );
            treeItem->setData( 0, Qt::UserRole,
                                  QVariant::fromValue( (void*)p_item ) );
            values += p_item;
            table->addTopLevelItem( treeItem );
        }
    }
    table->resizeColumnToContents( 0 );

    CONNECT( table, itemDoubleClicked( QTreeWidgetItem *, int ),
             this, selectKey( QTreeWidgetItem * ) );
}

void KeySelectorControl::selectKey( QTreeWidgetItem *keyItem )
{
   module_config_t *p_keyItem = static_cast<module_config_t*>
                          (keyItem->data( 0, Qt::UserRole ).value<void*>());

    KeyInputDialog *d = new KeyInputDialog( values, p_keyItem->psz_text );
    d->exec();
    if( d->result() == QDialog::Accepted )
    {
        p_keyItem->value.i = d->keyValue;
        if( d->conflicts )
        {
            for( int i = 0; i < table->topLevelItemCount() ; i++ )
            {
                QTreeWidgetItem *it = table->topLevelItem(i);
                module_config_t *p_item = static_cast<module_config_t*>
                              (it->data( 0, Qt::UserRole ).value<void*>());
                if( p_keyItem != p_item && p_item->value.i == d->keyValue )
                    p_item->value.i = 0;
                it->setText( 1, VLCKeyToString( p_item->value.i ) );
            }
        }
        else
            keyItem->setText( 1, VLCKeyToString( p_keyItem->value.i ) );
    }
    delete d;
}

void KeySelectorControl::doApply()
{
    foreach( module_config_t *p_current, values )
    {
        config_PutInt( p_this, p_current->psz_name, p_current->value.i );
    }
}

KeyInputDialog::KeyInputDialog( QList<module_config_t*>& _values,
                                const char * _keyToChange ) :
                                                QDialog(0), keyValue(0)
{
    setModal( true );
    values = _values;
    conflicts = false;
    keyToChange = _keyToChange;
    setWindowTitle( qtr( "Hotkey for " ) + qfu( keyToChange)  );

    QVBoxLayout *l = new QVBoxLayout( this );
    selected = new QLabel( qtr("Press the new keys for ")  + qfu(keyToChange) );
    warning = new QLabel();
    l->addWidget( selected , Qt::AlignCenter );
    l->addWidget( warning, Qt::AlignCenter );

    QHBoxLayout *l2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton( qtr("OK") );
    l2->addWidget( ok );
    QPushButton *cancel = new QPushButton( qtr("Cancel") );
    l2->addWidget( cancel );

    BUTTONACT( ok, accept() );
    BUTTONACT( cancel, reject() );

    l->addLayout( l2 );
}

void KeyInputDialog::checkForConflicts( int i_vlckey )
{
    conflicts = false;
    module_config_t *p_current = NULL;
    foreach( p_current, values )
    {
        if( p_current->value.i == i_vlckey && strcmp( p_current->psz_text,
                                                    keyToChange ) )
        {
            conflicts = true;
            break;
        }
    }
    if( conflicts )
    {
        warning->setText(
          qtr("Warning: the  key is already assigned to \"") +
          QString( p_current->psz_text ) + "\"" );
    }
    else warning->setText( "" );
}

void KeyInputDialog::keyPressEvent( QKeyEvent *e )
{
    if( e->key() == Qt::Key_Tab ) return;
    int i_vlck = qtEventToVLCKey( e );
    selected->setText( VLCKeyToString( i_vlck ) );
    checkForConflicts( i_vlck );
    keyValue = i_vlck;
}

void KeyInputDialog::wheelEvent( QWheelEvent *e )
{
    int i_vlck = qtWheelEventToVLCKey( e );
    selected->setText( VLCKeyToString( i_vlck ) );
    checkForConflicts( i_vlck );
    keyValue = i_vlck;
}
