#include "colordialog.h"


Label::Label(QWidget *parent) : QLabel(parent)
{
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
}
  
void Label::mousePressEvent(QMouseEvent*)
{
  emit clicked();
}
  
ColorWidget::ColorWidget(int index, QColor color, QWidget *parent) : QWidget(parent)
{
  QLabel *indexLabel = new QLabel("Index " + QString::number(index) + ":");
  m_label = new Label;
  m_label->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

  setColor(color);
  
  QHBoxLayout *layout = new QHBoxLayout;
  this->setLayout(layout);
  
  layout->addWidget(indexLabel);
  layout->addWidget(m_label);
  
  connect(m_label,SIGNAL(clicked()),this,SLOT(getColor()));
}
  
QColor ColorWidget::color()
{
  return m_color;
}
  
void ColorWidget::setColor(QColor color)
{
  m_color = color;
  m_label->setText(color.name() + " QColor(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + ")");
  m_label->setPalette(QPalette(color));
  m_label->setAutoFillBackground(true);
}

void ColorWidget::getColor()
{
  QColor color = QColorDialog::getColor(m_color, this, "Select Color");
  if(color.isValid()){
    setColor(color);
  }
}

ColorDialog::ColorDialog(QList<QColor> colors, QWidget* parent): QDialog(parent), m_colors(colors)
{
  this->setWindowTitle(tr("Settings"));
  
  QVBoxLayout *layout = new QVBoxLayout;
  this->setLayout(layout);
  
  colorLayout = new QVBoxLayout;
  layout->addLayout(colorLayout);
  
  for(int i=0;i<colors.size();i++){
    ColorWidget *widget = new ColorWidget(i+1,colors.at(i));
    m_colorWidgets << widget;
    colorLayout->addWidget( widget );
  }
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  layout->addWidget(buttonBox);
  
  connect(buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
  connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));
}

QList<QColor> ColorDialog::getListColor()
{
  m_colors.clear();
  foreach(ColorWidget* color, m_colorWidgets){
    m_colors << color->color();
  }
  return m_colors;
}