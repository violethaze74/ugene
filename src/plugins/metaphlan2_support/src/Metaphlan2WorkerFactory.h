/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2018 UniPro <ugene@unipro.ru>
 * http://ugene.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_METAPHLAN2_WORKER_FACTORY_H_
#define _U2_METAPHLAN2_WORKER_FACTORY_H_

#include <U2Lang/LocalDomain.h>

namespace U2 {
namespace LocalWorkflow {

class Metaphlan2WorkerFactory : public DomainFactory {
    Q_DECLARE_TR_FUNCTIONS(Metaphlan2WorkerFactory)
public:
    Metaphlan2WorkerFactory();

    Worker *createWorker(Actor *actor);

    static void init();
    static void cleanup();

    static const QString ACTOR_ID;

    static const QString INPUT_PORT_ID;

    static const QString INPUT_SLOT;
    static const QString PAIRED_INPUT_SLOT;

    static const QString SEQUENCING_READS;
    static const QString INPUT_FORMAT;
    static const QString DB_URL;
    static const QString NUM_THREADS;
    static const QString BOWTIE2_OUTPUT_URL;
    static const QString OUTPUT_URL;

    static const QString SINGLE_END_TEXT;
    static const QString PAIRED_END_TEXT;

    static const QString INPUT_TYPE_FASTA;
    static const QString INPUT_TYPE_FASTQ;

    static const QString SINGLE_END;
    static const QString PAIRED_END;
};

}   // namespace LocalWorkflow
}   // namespace U2

#endif // _U2_METAPHLAN2_WORKER_FACTORY_H_