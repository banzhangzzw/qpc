/*$file${src::qs::qutest.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: qpc.qm
* File:  ${src::qs::qutest.c}
*
* This code has been generated by QM 5.2.2 <www.state-machine.com/qm>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This code is covered by the following QP license:
* License #    : LicenseRef-QL-dual
* Issued to    : Any user of the QP/C real-time embedded framework
* Framework(s) : qpc
* Support ends : 2023-12-31
* License scope:
*
* Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
*
* SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
*
* This software is dual-licensed under the terms of the open source GNU
* General Public License version 3 (or any later version), or alternatively,
* under the terms of one of the closed source Quantum Leaps commercial
* licenses.
*
* The terms of the open source GNU General Public License version 3
* can be found at: <www.gnu.org/licenses/gpl-3.0>
*
* The terms of the closed source Quantum Leaps commercial licenses
* can be found at: <www.state-machine.com/licensing>
*
* Redistributions in source code must retain this top-level comment block.
* Plagiarizing this software to sidestep the license obligations is illegal.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*/
/*$endhead${src::qs::qutest.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*! @file
* @brief QUTest unit testing harness
*/

/* Include this content in the build only when Q_UTEST is defined */
#ifdef Q_UTEST

#define QP_IMPL       /* this is QP implementation */
#include "qf_port.h"  /* QF port */
#include "qf_pkg.h"   /* QF package-scope interface */
#include "qassert.h"  /* QP embedded systems-friendly assertions */
#include "qs_port.h"  /* include QS port */
#include "qs_pkg.h"   /* QS facilities for pre-defined trace records */

/*==========================================================================*/
/* QUTest unit testing harness */
/*$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/* Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*$define${QUTest} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/*${QUTest::QS::testData} ..................................................*/
struct QS_TestData QS_testData;

/*${QUTest::QS::test_pause_} ...............................................*/
void QS_test_pause_(void) {
    QS_beginRec_((uint_fast8_t)QS_TEST_PAUSED);
    QS_endRec_();
    QS_onTestLoop();
}

/*${QUTest::QS::getTestProbe_} .............................................*/
uint32_t QS_getTestProbe_(QSpyFunPtr api) {
    uint32_t data = 0U;
    uint_fast8_t i;
    for (i = 0U; i < QS_testData.tpNum; ++i) {
        uint_fast8_t j;

        if (QS_testData.tpBuf[i].addr == (QSFun)api) {
            QS_CRIT_STAT_

            data = QS_testData.tpBuf[i].data;

            QS_CRIT_E_();
            QS_beginRec_((uint_fast8_t)QS_TEST_PROBE_GET);
                QS_TIME_PRE_();    /* timestamp */
                QS_FUN_PRE_(api);  /* the calling API */
                QS_U32_PRE_(data); /* the Test-Probe data */
            QS_endRec_();
            QS_CRIT_X_();

            QS_REC_DONE(); /* user callback (if defined) */

            --QS_testData.tpNum; /* one less Test-Probe */
            /* move all remaining entries in the buffer up by one */
            for (j = i; j < QS_testData.tpNum; ++j) {
                QS_testData.tpBuf[j] = QS_testData.tpBuf[j + 1U];
            }
            break; /* we are done (Test-Probe retreived) */
        }
    }
    return data;
}
/*$enddef${QUTest} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*..........................................................................*/
Q_NORETURN Q_onAssert(
    char const * module,
    int_t location)
{
    QS_BEGIN_NOCRIT_PRE_(QS_ASSERT_FAIL, 0U)
        QS_TIME_PRE_();
        QS_U16_PRE_(location);
        QS_STR_PRE_((module != (char *)0) ? module : "?");
    QS_END_NOCRIT_PRE_()

    QS_onFlush(); /* flush the assertion record to the host */
    QS_onTestLoop(); /* loop to wait for commands (typically reset) */
    QS_onReset(); /* in case the QUTEST loop ever returns, reset manually */
    for (;;) { /* QS_onReset() should not return, but to ensure no-return */
    }
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) {
    return (++QS_testData.testTime);
}

/*==========================================================================*/
/* QP-stub for QUTest
* NOTE: The QP-stub is needed for unit testing QP applications, but might
* NOT be needed for testing QP itself. In that case, the build process
* can define Q_UTEST=0 to exclude the QP-stub from the build.
*/
#if Q_UTEST != 0

Q_DEFINE_THIS_MODULE("qutest")

/*$define${QUTest-stub} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/*${QUTest-stub::QS::processTestEvts_} .....................................*/
void QS_processTestEvts_(void) {
    QS_TEST_PROBE_DEF(&QS_processTestEvts_)

    /* return immediately (do nothing) for Test Probe != 0 */
    QS_TEST_PROBE(return;)

    while (QPSet_notEmpty(&QF_readySet_)) {
        uint_fast8_t const p = QPSet_findMax(&QF_readySet_);
        QActive * const a = QActive_registry_[p];

        /* perform the run-to-completion (RTC) step...
        * 1. retrieve the event from the AO's event queue, which by this
        *    time must be non-empty and The "Vanialla" kernel asserts it.
        * 2. dispatch the event to the AO's state machine.
        * 3. determine if event is garbage and collect it if so
        */
        QEvt const * const e = QActive_get_(a);
        QHSM_DISPATCH(&a->super, e, a->prio);
    #if (QF_MAX_EPOOL > 0U)
        QF_gc(e);
    #endif
        if (a->eQueue.frontEvt == (QEvt *)0) { /* empty queue? */
            QPSet_remove(&QF_readySet_, p);
        }
    }
}

/*${QUTest-stub::QF::init} .................................................*/
void QF_init(void) {
    /* Clear the internal QF variables, so that the framework can start
    * correctly even if the startup code fails to clear the uninitialized
    * data (as is required by the C Standard).
    */
    QF_maxPool_ = 0U;
    QF_intNest_ = 0U;

    QF_bzero(&QActive_registry_[0], sizeof(QActive_registry_));
    QF_bzero(&QF_readySet_,         sizeof(QF_readySet_));
}

/*${QUTest-stub::QF::stop} .................................................*/
void QF_stop(void) {
    QS_onReset();
}

/*${QUTest-stub::QF::run} ..................................................*/
int_t QF_run(void) {
    /* function dictionaries for the standard API */
    QS_FUN_DICTIONARY(&QActive_post_);
    QS_FUN_DICTIONARY(&QActive_postLIFO_);
    QS_FUN_DICTIONARY(&QS_processTestEvts_);

    /* produce the QS_QF_RUN trace record */
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QF_RUN, 0U)
    QS_END_PRE_()

    QS_onTestLoop(); /* run the test loop */
    QS_onCleanup();  /* application cleanup */
    return 0; /* return no error */
}

/*${QUTest-stub::QActive} ..................................................*/

/*${QUTest-stub::QActive::start_} ..........................................*/
void QActive_start_(QActive * const me,
    QPrioSpec const prioSpec,
    QEvt const * * const qSto,
    uint_fast16_t const qLen,
    void * const stkSto,
    uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); /* QF-priority of the AO */
    me->pthre = (uint8_t)(prioSpec >> 8U);   /* preemption-threshold */
    QActive_register_(me); /* make QF aware of this active object */

    QEQueue_init(&me->eQueue, qSto, qLen); /* initialize the built-in queue */

    QHSM_INIT(&me->super, par, me->prio); /* the top-most initial tran. */
}

/*${QUTest-stub::QActive::stop} ............................................*/
#ifdef QF_ACTIVE_STOP
void QActive_stop(QActive * const me) {
    QActive_unsubscribeAll(me); /* unsubscribe from all events */
    QActive_unregister_(me); /* un-register this active object */
}
#endif /* def QF_ACTIVE_STOP */

/*${QUTest-stub::QTimeEvt} .................................................*/

/*${QUTest-stub::QTimeEvt::tick1_} .........................................*/
void QTimeEvt_tick1_(
    uint_fast8_t const tickRate,
    void const * const sender)
{
    QF_CRIT_STAT_
    QF_CRIT_E_();

    QTimeEvt *prev = &QTimeEvt_timeEvtHead_[tickRate];

    QS_BEGIN_NOCRIT_PRE_(QS_QF_TICK, 0U)
        ++prev->ctr;
        QS_TEC_PRE_(prev->ctr); /* tick ctr */
        QS_U8_PRE_(tickRate);   /* tick rate */
    QS_END_NOCRIT_PRE_()

    /* is current Time Event object provided? */
    QTimeEvt *t = (QTimeEvt *)QS_rxPriv_.currObj[TE_OBJ];
    if (t != (QTimeEvt *)0) {

        /* the time event must be armed */
        Q_ASSERT_ID(810, t->ctr != 0U);

        /* temp. for volatile */
        QActive * const act = (QActive * const)(t->act);

        /* the recipient AO must be provided */
        Q_ASSERT_ID(820, act != (QActive *)0);

        /* periodic time evt? */
        if (t->interval != 0U) {
            t->ctr = t->interval; /* rearm the time event */
        }
        else { /* one-shot time event: automatically disarm */
            t->ctr = 0U; /* auto-disarm */
            /* mark time event 't' as NOT linked */
            t->super.refCtr_ &= (uint8_t)(~(uint8_t)QTE_IS_LINKED);

            QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_AUTO_DISARM, act->prio)
                QS_OBJ_PRE_(t);        /* this time event object */
                QS_OBJ_PRE_(act);      /* the target AO */
                QS_U8_PRE_(tickRate);  /* tick rate */
            QS_END_NOCRIT_PRE_()
        }

        QS_BEGIN_NOCRIT_PRE_(QS_QF_TIMEEVT_POST, act->prio)
            QS_TIME_PRE_();            /* timestamp */
            QS_OBJ_PRE_(t);            /* the time event object */
            QS_SIG_PRE_(t->super.sig); /* signal of this time event */
            QS_OBJ_PRE_(act);          /* the target AO */
            QS_U8_PRE_(tickRate);      /* tick rate */
        QS_END_NOCRIT_PRE_()

        QF_CRIT_X_(); /* exit critical section before posting */

        QACTIVE_POST(act, &t->super, sender); /* asserts if queue overflows */

        QF_CRIT_E_();
    }

    /* update the linked list of time events */
    for (;;) {
        t = prev->next;  /* advance down the time evt. list */

        /* end of the list? */
        if (t == (QTimeEvt *)0) {

            /* any new time events armed since the last QTimeEvt_tick_()? */
            if (QTimeEvt_timeEvtHead_[tickRate].act != (void *)0) {

                /* sanity check */
                Q_ASSERT_CRIT_(830, prev != (QTimeEvt *)0);
                prev->next = (QTimeEvt *)QTimeEvt_timeEvtHead_[tickRate].act;
                QTimeEvt_timeEvtHead_[tickRate].act = (void *)0;
                t = prev->next;  /* switch to the new list */
            }
            else {
                break; /* all currently armed time evts. processed */
            }
        }

        /* time event scheduled for removal? */
        if (t->ctr == 0U) {
            prev->next = t->next;
            /* mark time event 't' as NOT linked */
            t->super.refCtr_ &= (uint8_t)(~(uint8_t)QTE_IS_LINKED);
            /* do NOT advance the prev pointer */
            QF_CRIT_X_(); /* exit crit. section to reduce latency */

            /* prevent merging critical sections, see NOTE1 below  */
            QF_CRIT_EXIT_NOP();
        }
        else {
            prev = t; /* advance to this time event */
            QF_CRIT_X_(); /* exit crit. section to reduce latency */

            /* prevent merging critical sections, see NOTE1 below  */
            QF_CRIT_EXIT_NOP();
        }
        QF_CRIT_E_(); /* re-enter crit. section to continue */
    }

    QF_CRIT_X_();

}

/*${QUTest-stub::QHsmDummy} ................................................*/

/*${QUTest-stub::QHsmDummy::ctor} ..........................................*/
void QHsmDummy_ctor(QHsmDummy * const me) {
    static struct QHsmVtable const vtable = {  /* QHsm virtual table */
        &QHsmDummy_init_,
        &QHsmDummy_dispatch_
    #ifdef Q_SPY
        ,&QHsm_getStateHandler_
    #endif
    };
    /* superclass' ctor */
    QHsm_ctor(&me->super, Q_STATE_CAST(0));
    me->super.vptr = &vtable;  /* hook the vptr */
}

/*${QUTest-stub::QHsmDummy::init_} .........................................*/
void QHsmDummy_init_(
    QHsm * const me,
    void const * const par,
    uint_fast8_t const qs_id)
{
    Q_UNUSED_PAR(par);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
        QS_OBJ_PRE_(me);        /* this state machine object */
        QS_FUN_PRE_(me->state.fun); /* the source state */
        QS_FUN_PRE_(me->temp.fun);  /* the target of the initial transition */
    QS_END_PRE_()
}

/*${QUTest-stub::QHsmDummy::dispatch_} .....................................*/
void QHsmDummy_dispatch_(
    QHsm * const me,
    QEvt const * const e,
    uint_fast8_t const qs_id)
{
    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_DISPATCH, qs_id)
        QS_TIME_PRE_();             /* time stamp */
        QS_SIG_PRE_(e->sig);        /* the signal of the event */
        QS_OBJ_PRE_(me);            /* this state machine object */
        QS_FUN_PRE_(me->state.fun); /* the current state */
    QS_END_PRE_()
}

/*${QUTest-stub::QActiveDummy} .............................................*/

/*${QUTest-stub::QActiveDummy::ctor} .......................................*/
void QActiveDummy_ctor(QActiveDummy * const me) {
    static QActiveVtable const vtable = {  /* QActive virtual table */
        { &QActiveDummy_init_,
          &QActiveDummy_dispatch_
    #ifdef Q_SPY
          ,&QHsm_getStateHandler_
    #endif
        },
        &QActiveDummy_start_,
        &QActiveDummy_post_,
        &QActiveDummy_postLIFO_
    };
    /* superclass' ctor */
    QActive_ctor(&me->super, Q_STATE_CAST(0));
    me->super.super.vptr = &vtable.super;  /* hook the vptr */
}

/*${QUTest-stub::QActiveDummy::init_} ......................................*/
void QActiveDummy_init_(
    QHsm * const me,
    void const * const par,
    uint_fast8_t const qs_id)
{
    (void)qs_id; /* unused parameter */

    QHsmDummy_init_(me, par, ((QActive const *)me)->prio);
}

/*${QUTest-stub::QActiveDummy::dispatch_} ..................................*/
void QActiveDummy_dispatch_(
    QHsm * const me,
    QEvt const * const e,
    uint_fast8_t const qs_id)
{
    (void)qs_id; /* unused parameter */

    QHsmDummy_dispatch_(me, e, ((QActive const *)me)->prio);
}

/*${QUTest-stub::QActiveDummy::start_} .....................................*/
void QActiveDummy_start_(
    QActive * const me,
    QPrioSpec const prioSpec,
    QEvt const * * const qSto,
    uint_fast16_t const qLen,
    void * const stkSto,
    uint_fast16_t const stkSize,
    void const * const par)
{
    /* No special preconditions for checking parameters to allow starting
    * dummy AOs the exact same way as the real counterparts.
    */
    Q_UNUSED_PAR(qSto);
    Q_UNUSED_PAR(qLen);
    Q_UNUSED_PAR(stkSto);
    Q_UNUSED_PAR(stkSize);

    me->prio  = (uint8_t)(prioSpec & 0xFFU); /* QF-priority of the AO */
    me->pthre = (uint8_t)(prioSpec >> 8U);   /* preemption-threshold */
    QActive_register_(me); /* make QF aware of this active object */

    /* the top-most initial tran. (virtual) */
    QHSM_INIT(&me->super, par, me->prio);
    //QS_FLUSH();
}

/*${QUTest-stub::QActiveDummy::post_} ......................................*/
bool QActiveDummy_post_(
    QActive * const me,
    QEvt const * const e,
    uint_fast16_t const margin,
    void const * const sender)
{
    QS_TEST_PROBE_DEF(&QActive_post_)

    /* test-probe#1 for faking queue overflow */
    bool status = true;
    QS_TEST_PROBE_ID(1,
        status = false;
        if (margin == QF_NO_MARGIN) {
            /* fake assertion Mod=qf_actq,Loc=110 */
            Q_onAssert("qf_actq", 110);
        }
    )

    QF_CRIT_STAT_
    QF_CRIT_E_();

    /* is it a dynamic event? */
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); /* increment the reference counter */
    }

    uint_fast8_t const rec = (status ? (uint_fast8_t)QS_QF_ACTIVE_POST
                             : (uint_fast8_t)QS_QF_ACTIVE_POST_ATTEMPT);
    QS_BEGIN_NOCRIT_PRE_(rec, me->prio)
        QS_TIME_PRE_();      /* timestamp */
        QS_OBJ_PRE_(sender); /* the sender object */
        QS_SIG_PRE_(e->sig); /* the signal of the event */
        QS_OBJ_PRE_(me);     /* this active object */
        QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & refCtr of the evt */
        QS_EQC_PRE_(0U);     /* number of free entries */
        QS_EQC_PRE_(margin); /* margin requested */
    QS_END_NOCRIT_PRE_()

    /* callback to examine the posted event under the same conditions
    * as producing the #QS_QF_ACTIVE_POST trace record, which are:
    * the local filter for this AO ('me->prio') is set
    */
    if ((QS_priv_.locFilter[me->prio >> 3U]
         & (1U << (me->prio & 7U))) != 0U)
    {
        QS_onTestPost(sender, me, e, status);
    }
    QF_CRIT_X_();

    /* recycle the event immediately, because it was not really posted */
    #if (QF_MAX_EPOOL > 0U)
    QF_gc(e);
    #endif

    return status; /* the event is "posted" correctly */
}

/*${QUTest-stub::QActiveDummy::postLIFO_} ..................................*/
void QActiveDummy_postLIFO_(
    QActive * const me,
    QEvt const * const e)
{
    QS_TEST_PROBE_DEF(&QActive_postLIFO_)

    /* test-probe#1 for faking queue overflow */
    QS_TEST_PROBE_ID(1,
        /* fake assertion Mod=qf_actq,Loc=210 */
        Q_onAssert("qf_actq", 210);
    )

    QF_CRIT_STAT_
    QF_CRIT_E_();

    /* is it a dynamic event? */
    if (e->poolId_ != 0U) {
        QF_EVT_REF_CTR_INC_(e); /* increment the reference counter */
    }

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_POST_LIFO, me->prio)
        QS_TIME_PRE_();      /* timestamp */
        QS_SIG_PRE_(e->sig); /* the signal of this event */
        QS_OBJ_PRE_(me);     /* this active object */
        QS_2U8_PRE_(e->poolId_, e->refCtr_); /* pool Id & refCtr of the evt */
        QS_EQC_PRE_(0U);     /* number of free entries */
        QS_EQC_PRE_(0U);     /* min number of free entries */
    QS_END_NOCRIT_PRE_()

    /* callback to examine the posted event under the same conditions
    * as producing the #QS_QF_ACTIVE_POST trace record, which are:
    * the local filter for this AO ('me->prio') is set
    */
    if ((QS_priv_.locFilter[me->prio >> 3U]
         & (1U << (me->prio & 7U))) != 0U)
    {
        QS_onTestPost((QActive *)0, me, e, true);
    }

    QF_CRIT_X_();

    /* recycle the event immediately, because it was not really posted */
    #if (QF_MAX_EPOOL > 0U)
    QF_gc(e);
    #endif
}
/*$enddef${QUTest-stub} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#endif /* Q_UTEST != 0 */

#endif /* Q_UTEST */
