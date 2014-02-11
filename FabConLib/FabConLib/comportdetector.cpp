#include "comportdetector.h"
#include "qextserialenumerator.h"
#include "qextserialport.h"

#include <QDebug>

ComportDetector::ComportDetector()
{
    qDebug("\nworking");
    foreach(QextPortInfo port, QextSerialEnumerator::getPorts())
        {
            // Display the port's FriendName in the GUI portBox;
            // printer connection uses the PortName (portList values).
            // Ports which are clearly not printers (empty FriendNames) are ignored.

            // NOTE: Since the selected port name is retrieved by its combobox index
            //       (because friend name may not be unique), it is important that
            //       the order of portBox items be maintained.
            // The selected port name is retrieved by portList.at(ui->portBox->currentIndex()).


            if (!port.friendName.isEmpty())
            {
                qDebug("\n%s",port.friendName.toStdString().c_str());
                qDebug("\n%s",port.portName.toStdString().c_str());
            }
        }

}

